from explore import dora_
import io
import pandas as pd
from PIL import Image
import json
from io import BytesIO
import sys
import psycopg2
import pygraphviz as pgv
import os
import dotenv
from PyQt5.QtSql import QSqlQueryModel, QSqlDatabase
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QPushButton, QTextEdit, QLabel,
                             QMessageBox, QGraphicsView, QGraphicsScene, QTabWidget, QCheckBox, QButtonGroup,
                             QTableView, QProgressBar)
from PyQt5.QtGui import QPixmap, QImage, QPainter
from PyQt5.QtCore import Qt
import PyQt6.QtCore
import PyQt6.QtSql
import PyQt6.QtWidgets
import PyQt6

# Load environment variables
dotenv.load_dotenv()

# Database connection details from environment variables
DB = os.getenv("DATABASE") if os.getenv("DATABASE") else "postgres"
USER = os.getenv("USER") if os.getenv("USER") else "postgres"
PASSWORD = os.getenv("PASSWORD") if os.getenv("PASSWORD") else "password"
HOST = os.getenv("HOST") if os.getenv("HOST") else "localhost"
PORT = os.getenv("PORT") if os.getenv("PORT") else 5432


def exectute_query(query):
    with psycopg2.connect(dbname=DB, user=USER, password=PASSWORD, host=HOST, port=PORT) as con, con.cursor() as cursor:
        cursor.execute(query)
        result = cursor.fetchall()
    return result


def retrieve_qep(sql):
    sql_exp = "EXPLAIN (ANALYZE, COSTS, VERBOSE, BUFFERS, FORMAT JSON) " + sql
    qep = exectute_query(sql_exp)
    while not isinstance(qep, dict):
        qep = qep[0]
    return qep


def retrieve_result(sql, columns):
    pass


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
    results = {"node": {}, "vertices": set(), "edges": set(), "sql": {}}
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
        "Total Reads": node.get("Shared Read Blocks"),
        "Reads from Cache": node.get("Shared Hit Blocks"),
        "Disk Reads": abs(node.get("Shared Read Blocks") - node.get("Shared Hit Blocks")),
    }

    detailString = f"{details['Node Type']}\nRows: {details['Actual Rows']}\nTime: {details['Actual Total Time']}\n" \
                   f"Cost: {details['Total Cost']}\nTotal Reads': {details['Total Reads']}\n"\
                    f"Reads from Cache: {details['Reads from Cache']}\n" \
                   f"Disk Reads: {details['Disk Reads']}"
    details["string"] = detailString

    return details


def plot_qep(results):
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
            attrs['fillcolor'] = '#e3d2c1'
        elif 'Join' in vertex:
            attrs['shape'] = 'diamond'
            attrs['fillcolor'] = '#add8e6'
        elif 'Sort' in vertex:
            attrs['shape'] = 'trapezium'
            attrs['fillcolor'] = '#ffa07a'
        elif 'Aggregate' in vertex:
            attrs['shape'] = 'rectangle'
            attrs['fillcolor'] = '#caf8e6'
        elif 'Filter' in vertex:
            attrs['shape'] = 'hexagon'
            attrs['fillcolor'] = '#faf00a'
        elif 'Hash' in vertex:
            attrs['shape'] = 'octagon'
            attrs['fillcolor'] = '#caff9a'

        G.add_node(vertex, **attrs)

    for edge in results['edges']:
        G.add_edge(edge[0], edge[1])

    G.graph_attr['rankdir'] = 'TB'
    G.node_attr['shape'] = 'box'
    G.edge_attr['color'] = 'black'

    G.layout(prog='dot')

    buffer = io.BytesIO()
    G.draw(buffer, format='png')
    buffer.seek(0)
    return buffer


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


"""SEPARATOR HERE"""
from PyQt5 import QtCore, QtGui, QtQml
import numpy as np
import pandas as pd


class DataFrameModel(QtCore.QAbstractTableModel):
    DtypeRole = QtCore.Qt.UserRole + 1000
    ValueRole = QtCore.Qt.UserRole + 1001

    def __init__(self, df=pd.DataFrame(), parent=None):
        super(DataFrameModel, self).__init__(parent)
        self._dataframe = df

    def setDataFrame(self, dataframe):
        self.beginResetModel()
        self._dataframe = dataframe.copy()
        self.endResetModel()

    def dataFrame(self):
        return self._dataframe

    dataFrame = QtCore.pyqtProperty(pd.DataFrame, fget=dataFrame, fset=setDataFrame)

    @QtCore.pyqtSlot(int, QtCore.Qt.Orientation, result=str)
    def headerData(self, section: int, orientation: QtCore.Qt.Orientation, role: int = QtCore.Qt.DisplayRole):
        if role == QtCore.Qt.DisplayRole:
            if orientation == QtCore.Qt.Horizontal:
                return self._dataframe.columns[section]
            else:
                return str(self._dataframe.index[section])
        return QtCore.QVariant()

    def rowCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return len(self._dataframe.index)

    def columnCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return self._dataframe.columns.size

    def data(self, index, role=QtCore.Qt.DisplayRole):
        if not index.isValid() or not (0 <= index.row() < self.rowCount() \
                                       and 0 <= index.column() < self.columnCount()):
            return QtCore.QVariant()

        row = index.row()
        col = index.column()
        val = self._dataframe.iloc[row, col]

        if role == QtCore.Qt.DisplayRole:
            return str(val)
        elif role == DataFrameModel.ValueRole:
            return val
        if role == DataFrameModel.DtypeRole:
            dt = self._dataframe.dtypes[col]
            return dt
        return QtCore.QVariant()

    def roleNames(self):
        roles = {
            QtCore.Qt.DisplayRole: b'display',
            DataFrameModel.DtypeRole: b'dtype',
            DataFrameModel.ValueRole: b'value'
        }
        return roles


from PyQt5.QtWidgets import (
    QApplication, QTableView, QVBoxLayout, QWidget, QGroupBox,
    QLabel, QScrollArea, QPushButton, QErrorMessage, QToolButton, QTabWidget
)
from PyQt5.QtCore import Qt, QPropertyAnimation, QEasingCurve, pyqtSignal
import sys
import re


class DataFrameTab(QTabWidget):
    def __init__(self, data_frame_getters, names):
        super().__init__()

        for get_df, name in zip(data_frame_getters, names):
            df = get_df()
            view = QTableView()
            model = DataFrameModel(df)
            view.setModel(model)
            view.setStyleSheet("""
                QTableView {
                    selection-background-color: #E0E0E0;
                    font: 11pt "Calibri";
                }
                QHeaderView::section {
                    background-color: #F0F0F0;
                    padding: 4px;
                    border: 1px solid #D0D0D0;
                    font-size: 10pt;
                }
            """)
            self.addTab(view, name)

class QEPVisualizer(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.initSecondTab()

    def initUI(self):
        self.setWindowTitle('QEP Visualizer')
        self.setGeometry(100, 100, 1200, 600)

        # Central widget and layout
        centralWidget = QWidget(self)
        self.setCentralWidget(centralWidget)
        layout = QHBoxLayout(centralWidget)

        # Left side layout for the SQL query input and buttons
        leftLayout = QVBoxLayout()
        self.queryText = QTextEdit()
        self.generateButton = QPushButton('Generate QEP')
        self.generateButton.clicked.connect(self.generate_qep)
        leftLayout.addWidget(self.queryText)
        leftLayout.addWidget(self.generateButton)

        # Buttons for switching tabs
        self.switchButton1 = QPushButton('View QEP')
        self.switchButton2 = QPushButton('View Results')
        self.switchButton3 = QPushButton('View Intermediate Results')

        self.intermediateResultsWidget = QTextEdit()
        self.intermediateResultsWidget.setStyleSheet("font-size: 12px; font-family: Courier New;")
        self.intermediateResultsWidget.setAlignment(Qt.AlignJustify)
        self.intermediateResultsWidget.setReadOnly(True)

        leftLayout.addWidget(self.switchButton1)
        leftLayout.addWidget(self.switchButton2)
        leftLayout.addWidget(self.switchButton3)

        layout.addLayout(leftLayout)

        # Right side for the tabbed image viewers
        self.tabs = QTabWidget()
        self.scene1 = QGraphicsScene()
        self.view1 = GraphicsView(self.scene1)

        self.view2 = QTableView()

        self.tabs.addTab(self.view1, "QEP 1")
        self.tabs.addTab(self.view2, "Results")
        self.tabs.addTab(self.intermediateResultsWidget, "Intermediate Results")

        layout.addWidget(self.tabs)

        # Buttons to change
        self.switchButton1.clicked.connect(lambda: self.switch_tab(0))
        self.switchButton2.clicked.connect(lambda: self.switch_tab(1))
        self.switchButton3.clicked.connect(lambda: self.switch_tab(2))

        # Set the layout stretch
        layout.setStretch(0, 1)
        layout.setStretch(1, 2)

    def switch_tab(self, index):
        self.tabs.setCurrentIndex(index)

    def initSecondTab(self):
        # Generate a unique connection name
        connection_name = "ResultsConnection"
        if QSqlDatabase.contains(connection_name):
            self.db = QSqlDatabase.database(connection_name)
        else:
            self.db = QSqlDatabase.addDatabase("QPSQL", connection_name)

        self.db.setDatabaseName(DB)
        self.db.setPassword(PASSWORD)
        self.db.setUserName(USER)
        self.db.setHostName(HOST)
        self.db.setPort(int(PORT))

        if not self.db.open():
            QMessageBox.critical(self, "Database Error", self.db.lastError().text())
            return

        self.model = QSqlQueryModel()
        self.view2.setModel(self.model)

        # Execute the query and refresh the view
        self.run_query()

    def run_query(self):
        # Check if the database is open before running the query
        if self.db.isOpen():
            self.model.setQuery(self.queryText.toPlainText(), self.db)
            self.view2.setModel(self.model)  # Refresh the model
            self.view2.show()
        else:
            QMessageBox.critical(self, "Database Error", "Database is not open.")

    def display_intermediate_results(self, results_dict):
        formatted_text = ""
        for step_number, (step_description, step_data) in enumerate(results_dict.items(), start=1):
            # Step description
            step_info = step_description.split('\n')[0]
            formatted_text += f"{step_info}:\n"
            for __, _ in enumerate(step_data):
                _ = str(_)
                _.replace('\\n', '\n').replace("', '", "',\n'").replace("\\\\", "\\")
                if __ < len(step_data):
                    formatted_text += _ + "\n\n"
                    formatted_text += "---------------------------------------------------------------------\n"
                    l_replacer = lambda x: x.replace(
                        "---------------------------------------------------------------------\n" * 2,
                        "---------------------------------------------------------------------\n")
                    formatted_text = l_replacer(l_replacer(formatted_text))

        self.intermediateResultsWidget.setText(formatted_text)

    def generate_qep(self):
        sql = self.queryText.toPlainText()
        try:
            qep = retrieve_qep(sql)

            total_time = qep["Plan"]["Actual Total Time"]
            total_cost = qep["Plan"]["Total Cost"]
            total_rows = qep["Plan"]["Actual Rows"]

            results = breakdown(qep["Plan"], total_time, total_cost, total_rows)
            img_buffer = plot_qep(results)
            self.display_qep_image(img_buffer)
            QMessageBox.information(self, "Success", "QEP generated successfully.")

            self.run_query()

            d1 = dora_(self.queryText.toPlainText())

            self.display_intermediate_results(d1)
            data_frame_getters = [lambda df=item[0]: df for item in d1.values()]
            names = [re.compile(r"\d+").sub("", _) for _ in d1.keys()]

            for name, get_df in zip(names, data_frame_getters):
                df = get_df()
                view = QTableView()
                model = DataFrameModel(df)
                view.setModel(model)
                view.setStyleSheet("""
                    QTableView {
                        selection-background-color: #E0E0E0;
                        font: 11pt "Calibri";
                    }
                    QHeaderView::section {
                        background-color: #F0F0F0;
                        padding: 4px;
                        border: 1px solid #D0D0D0;
                        font-size: 10pt;
                    }
                """)
                self.tabs.addTab(view, name)

        except Exception as e:
            QMessageBox.critical(self, "Error", str(e))

    def display_qep_image(self, file):
        # Clear the previous scene
        self.scene1.clear()

        # Load the image from buffer and add it to the scene
        image = QImage()
        image.loadFromData(file.getvalue(), format='PNG')
        pixmap = QPixmap.fromImage(image)
        self.scene1.addPixmap(pixmap)
        self.view1.fitInView(self.scene1.itemsBoundingRect(), Qt.KeepAspectRatio)

        # Update the scene
        self.view1.setScene(self.scene1)


def main():
    app = QApplication(sys.argv)
    ex = QEPVisualizer()
    ex.show()
    sys.exit(app.exec())



