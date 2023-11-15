import json
import sys
import psycopg2
import pygraphviz as pgv
import os
import dotenv
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QPushButton, QTextEdit, QLabel,
                             QMessageBox, QGraphicsView, QGraphicsScene)
from PyQt5.QtGui import QPixmap, QImage, QPainter
from PyQt5.QtCore import Qt

# Load environment variables
dotenv.load_dotenv()

# Database connection details from environment variables
DB = os.getenv("DATABASE")
USER = os.getenv("USER")
PASSWORD = os.getenv("PASSWORD")
HOST = os.getenv("HOST") if os.getenv("HOST") else "localhost"


def retrieve_qep(sql):
    sql_exp = "EXPLAIN (ANALYZE, COSTS, VERBOSE, BUFFERS, FORMAT JSON) " + sql
    with psycopg2.connect(dbname=DB, user=USER, password=PASSWORD, host=HOST) as con, con.cursor() as cursor:
        cursor.execute(sql_exp)
        qep = cursor.fetchone()[0]
    return qep


def calculate_duration(node, total_time):
    duration = node.get("Actual Total Time", 0)
    return round((duration / total_time) * 100, 2) if total_time else 0


def calculate_cost(node, total_cost):
    cost = node.get("Total Cost", 0)
    return round((cost / total_cost) * 100, 2) if total_cost else 0


def calculate_rows_removed(node, total_rows):
    rows_removed = node.get("Rows Removed by Filter", 0)
    return round((rows_removed / total_rows) * 100, 2) if total_rows else 0


def breakdown(curNode, total_time, total_cost, total_rows):
    results = {"node": {}, "vertices": set(), "edges": set()}
    process_node(curNode, results, total_time, total_cost, total_rows)
    return results


def process_node(node, results, total_time, total_cost, total_rows):
    nodeDetails = get_node_details(node, total_time, total_cost, total_rows)
    results["node"][nodeDetails["string"]] = nodeDetails
    results["vertices"].add(nodeDetails["string"])

    if "Plans" in node:
        for child in node["Plans"]:
            results["edges"].add(
                (nodeDetails["string"], get_node_details(child, total_time, total_cost, total_rows)["string"]))
            process_node(child, results, total_time, total_cost, total_rows)


def get_node_details(node, total_time, total_cost, total_rows):
    duration_percent = calculate_duration(node, total_time)
    cost_percent = calculate_cost(node, total_cost)
    rows_removed_percent = calculate_rows_removed(node, total_rows)

    details = {
        "Node Type": node.get("Node Type"),
        "Actual Rows": node.get("Actual Rows"),
        "Actual Total Time": node.get("Actual Total Time"),
        "Total Cost": node.get("Total Cost"),
        "Duration Percent": duration_percent,
        "Cost Percent": cost_percent,
        "Rows Removed Percent": rows_removed_percent,
    }

    detailString = f"{details['Node Type']}\nRows: {details['Actual Rows']}\nTime: {details['Actual Total Time']}\n" \
                   f"Cost: {details['Total Cost']}\nDuration %: {details['Duration Percent']}\nCost %: {details['Cost Percent']}\n" \
                   f"Rows Removed %: {details['Rows Removed Percent']}"
    details["string"] = detailString
    with open("details.json", "w") as f:
        json.dump(details, f, indent=4)
    return details


def plot_qep(results, filename='qep.png'):
    G = pgv.AGraph(directed=True, strict=True, fontname='Helvetica', fontsize=12, dpi=400)

    for vertex in results['vertices']:
        attrs = {
            'shape': 'box',
            'style': 'filled',
            'fillcolor': 'white',
            'fontcolor': 'black',
            'fontsize': 10
        }
        if 'Scan' in vertex:
            attrs['shape'] = 'ellipse'
            attrs['fillcolor'] = '#d3d3d3'  # Light grey
        elif 'Join' in vertex:
            attrs['shape'] = 'diamond'
            attrs['fillcolor'] = '#add8e6'  # Light blue
        elif 'Sort' in vertex:
            attrs['shape'] = 'trapezium'
            attrs['fillcolor'] = '#ffa07a'  # Light salmon

        G.add_node(vertex, **attrs)

    for edge in results['edges']:
        G.add_edge(edge[0], edge[1])

    G.graph_attr['rankdir'] = 'TB'
    G.node_attr['shape'] = 'box'
    G.edge_attr['color'] = 'black'

    G.layout(prog='dot')
    G.draw(filename)
    return filename


class GraphicsView(QGraphicsView):
    def __init__(self, scene):
        super().__init__(scene)
        self.setScene(scene)
        self.setDragMode(QGraphicsView.ScrollHandDrag)
        self.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        # Set initial zoom scale
        self._zoom = 0

    def wheelEvent(self, event):
        # Zoom Factor
        zoom_in_factor = 1.25
        zoom_out_factor = 1 / zoom_in_factor

        # Set Anchors
        self.setTransformationAnchor(QGraphicsView.NoAnchor)
        self.setResizeAnchor(QGraphicsView.NoAnchor)

        # Save the scene pos
        old_pos = self.mapToScene(event.pos())

        # Zoom
        if event.angleDelta().y() > 0:
            zoom_factor = zoom_in_factor
            self._zoom += 1
        else:
            zoom_factor = zoom_out_factor
            self._zoom -= 1

        if self._zoom > 0:
            self.scale(zoom_factor, zoom_factor)
        elif self._zoom == 0:
            self.resetTransform()
        else:
            self._zoom = 0

        # Get the new position
        new_pos = self.mapToScene(event.pos())

        # Move scene to old position
        delta = new_pos - old_pos
        self.translate(delta.x(), delta.y())


class QEPVisualizer(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('QEP Visualizer')
        self.setGeometry(100, 100, 1200, 600)

        # Central widget and layout
        centralWidget = QWidget(self)
        self.setCentralWidget(centralWidget)
        layout = QHBoxLayout(centralWidget)

        # Left side layout for the SQL query input
        leftLayout = QVBoxLayout()
        self.queryText = QTextEdit()
        self.generateButton = QPushButton('Generate QEP')
        self.generateButton.clicked.connect(self.generate_qep)
        leftLayout.addWidget(self.queryText)
        leftLayout.addWidget(self.generateButton)
        layout.addLayout(leftLayout)

        # Right side for the image viewer
        self.scene = QGraphicsScene()
        self.view = GraphicsView(self.scene)
        layout.addWidget(self.view)

        # Set the layout stretch
        layout.setStretch(0, 1)
        layout.setStretch(1, 2)

    def generate_qep(self):
        sql = self.queryText.toPlainText()
        try:
            qep = retrieve_qep(sql)
            qep_dict = dict(qep[0])

            total_time = qep_dict["Plan"]["Actual Total Time"]
            total_cost = qep_dict["Plan"]["Total Cost"]
            total_rows = qep_dict["Plan"]["Actual Rows"]

            results = breakdown(qep[0]["Plan"], total_time, total_cost, total_rows)
            filename = plot_qep(results)
            self.display_qep_image(filename)
            QMessageBox.information(self, "Success", "QEP generated successfully.")
        except Exception as e:
            QMessageBox.critical(self, "Error", str(e))

    def display_qep_image(self, filename):
        # Clear the previous scene
        self.scene.clear()

        # Load the image and add it to the scene
        image = QImage(filename)
        pixmap = QPixmap.fromImage(image)
        self.scene.addPixmap(pixmap)
        self.view.fitInView(self.scene.itemsBoundingRect(), Qt.KeepAspectRatio)

        # Update the scene
        self.view.setScene(self.scene)


