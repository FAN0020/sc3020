import json
import tkinter as tk
from tkinter import messagebox
import psycopg2
import pandas as pd
import pygraphviz as pgv
import os
import dotenv
import pprint
from io import BytesIO
import json
import base64
import zlib

# Load environment variables
dotenv.load_dotenv()

# Database connection details from environment variables
DB = os.getenv("DATABASE")
USER = os.getenv("USER")
PASSWORD = os.getenv("PASSWORD")
HOST = os.getenv("HOST") if os.getenv("HOST") else "localhost"


# Function to retrieve the QEP
def retrieve_qep(sql):
    sql_exp = "EXPLAIN (ANALYZE, COSTS, VERBOSE, BUFFERS, FORMAT JSON) " + sql
    with psycopg2.connect(dbname=DB, user=USER, password=PASSWORD, host=HOST) as con, con.cursor() as cursor:
        cursor.execute(sql_exp)
        qep = cursor.fetchone()[0]
    return qep


def breakdown(curNode):
    results = {"node": {}, "vertices": set(), "edges": set()}
    process_node(curNode, results)
    return results


def process_node(node, results):
    nodeDetails = get_node_details(node)
    results["node"][nodeDetails["string"]] = nodeDetails
    results["vertices"].add(nodeDetails["string"])

    if "Plans" in node:
        for child in node["Plans"]:
            results["edges"].add((nodeDetails["string"], get_node_details(child)["string"]))
            process_node(child, results)


def get_node_details(node):
    details = {
        "Node Type": node.get("Node Type"),
        "Output": node.get("Output", []),
        "Actual Rows": node.get("Actual Rows"),
        "Actual Total Time": node.get("Actual Total Time"),
        "Shared Hit Blocks": node.get("Shared Hit Blocks", 0),
        "Shared Read Blocks": node.get("Shared Read Blocks", 0),
        "Local Hit Blocks": node.get("Local Hit Blocks", 0),
        "Local Read Blocks": node.get("Local Read Blocks", 0),
    }
    detailString = f"{details['Node Type']}\nRows: {details['Actual Rows']}\nTime: {details['Actual Total Time']}\n" \
                   f"Shared Hits: {details['Shared Hit Blocks']}\nShared Reads: {details['Shared Read Blocks']}\n" \
                   f"Local Hits: {details['Local Hit Blocks']}\nLocal Reads: {details['Local Read Blocks']}"
    details["string"] = detailString
    return details


def plot_qep(results, filename='qep.png'):
    G = pgv.AGraph(directed=True, strict=True, fontname='Helvetica', fontsize=12)

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


def generate_qep():
    sql = query_text.get("1.0", tk.END)
    try:
        qep = retrieve_qep(sql)
        # qep_dict = dict(qep[0])
        # with open("qep.json", "w") as wf:
        #     json.dump(qep_dict, wf, indent=4)
        # pprint.pprint(qep)
        results = breakdown(qep[0]["Plan"])
        filename = plot_qep(results)
        display_qep_image(filename)
        messagebox.showinfo("Success", "QEP generated successfully.")
    except Exception as e:
        messagebox.showerror("Error", str(e))


root = tk.Tk()
root.title("QEP Visualizer")

# Main frames for layout
left_frame = tk.Frame(root)
left_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

right_frame = tk.Frame(root)
right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

# Text box for entering SQL query with a Scrollbar
query_text = tk.Text(left_frame, height=20, width=60)
query_scroll = tk.Scrollbar(left_frame, command=query_text.yview)
query_text.configure(yscrollcommand=query_scroll.set)
query_scroll.pack(side=tk.RIGHT, fill=tk.Y)
query_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

# Frame to display the QEP image with Scrollbars
image_frame = tk.LabelFrame(right_frame, text="QEP Image", padx=5, pady=5)
image_frame.pack(fill=tk.BOTH, expand=True)

# Canvas for image scrolling
canvas = tk.Canvas(image_frame)
image_scroll_y = tk.Scrollbar(image_frame, orient="vertical", command=canvas.yview)
image_scroll_x = tk.Scrollbar(image_frame, orient="horizontal", command=canvas.xview)
canvas.configure(yscrollcommand=image_scroll_y.set, xscrollcommand=image_scroll_x.set)
image_scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
image_scroll_x.pack(side=tk.BOTTOM, fill=tk.X)
canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

# Label to show the image inside the canvas
qep_image_label = tk.Label(canvas)
canvas.create_window((0, 0), window=qep_image_label, anchor='nw')


def display_qep_image(filename):
    # Open the image file
    with Image.open(filename) as img:
        # Convert the image to a format Tkinter can use
        imgtk = ImageTk.PhotoImage(img)
        # Update the label with the new image
        qep_image_label.config(image=imgtk)
        # Keep a reference to the image
        qep_image_label.image = imgtk
        # Set the scrollregion to the size of the image
        canvas.config(scrollregion=canvas.bbox("all"))


# Button to generate the QEP
generate_button = tk.Button(root, text="Generate QEP", command=generate_qep)
generate_button.pack()

# Run the GUI event loop
root.mainloop()