import tkinter as tk
from tkinter import simpledialog, filedialog, Canvas
from PIL import Image, ImageTk
import json
import math
import re
import atexit

class CustomDialog(simpledialog.Dialog):
    def body(self, master):
        tk.Label(master, text="Type:").grid(row=0)
        tk.Label(master, text="Point Name:").grid(row=1)
        tk.Label(master, text="Tag:").grid(row=2)
        tk.Label(master, text="Rating:").grid(row=3)

        self.type_var = tk.StringVar(master)
        self.type_var.set("Intersection")  # 默认值
        self.type_options = ["Restaurant", "Door", "Intersection", "Store", "Toilet","Services_Center","Background","Scenic","Teaching_buildings","Office_buildings","Dormitories"]
        self.type_menu = tk.OptionMenu(master, self.type_var, *self.type_options)
        self.type_menu.grid(row=0, column=1)

        self.e1 = tk.Entry(master)
        self.e2 = tk.Entry(master)
        self.e3 = tk.Entry(master)
        self.e1.grid(row=1, column=1)
        self.e2.grid(row=2, column=1)
        self.e3.grid(row=3, column=1)
        return self.e1  # initial focus

    def apply(self):
        self.result = (self.type_var.get(), self.e1.get(), self.e2.get(), self.e3.get())

class GraphApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Graph Marker")
        self.canvas = Canvas(root)
        self.canvas.pack(expand=tk.YES, fill=tk.BOTH)
        self.image = None
        self.points = []
        self.edges = []
        self.adjList = {}
        self.selected_points = []
        self.graph_file_path = None

        self.load_image_button = tk.Button(root, text="Load Image", command=self.load_image)
        self.load_image_button.pack()
        self.load_graph_button = tk.Button(root, text="Load Graph", command=self.load_graph)
        self.load_graph_button.pack()
        self.save_button = tk.Button(root, text="Save JSON", command=self.save_json)
        self.save_button.pack()

        self.canvas.bind("<Button-1>", self.mark_point)
        self.canvas.bind("<Button-3>", self.select_point)
        self.canvas.bind("<Button-2>", self.delete_edge)

        atexit.register(self.on_exit)

    def load_image(self):
        file_path = filedialog.askopenfilename()
        if file_path:
            self.image = Image.open(file_path)
            self.image_width, self.image_height = self.image.size
            self.image_tk = ImageTk.PhotoImage(self.image)
            self.canvas.config(width=self.image_width, height=self.image_height)
            self.canvas.create_image(0, 0, anchor=tk.NW, image=self.image_tk)

    def mark_point(self, event):
        x, y = event.x, event.y
        dialog = CustomDialog(self.root)
        if dialog.result:
            point_type, point_name, tag, rating = dialog.result
            if not point_name:
                return
            if not tag:
                tag = "No tag"
            try:
                rating = float(rating)
            except ValueError:
                rating = 0.0  # 处理非数值输入

            point = {"name": point_name, "index": len(self.points), "x": int(x), "y": int(y), "type": point_type,
                     "rating": rating, "tag": tag}

            self.points.append(point)
            self.adjList[self.format_point(point)] = []
            self.canvas.create_oval(x - 4, y - 4, x + 4, y + 4, fill='red')
            self.canvas.create_text(x, y - 10, text=point_name, fill='red')

    def format_point(self, point):
        return json.dumps(point, indent=4)  # 直接使用JSON格式来格式化点信息

    def select_point(self, event):
        x, y = event.x, event.y
        closest_point = min(self.points, key=lambda p: (p['x'] - x) ** 2 + (p['y'] - y) ** 2)
        self.selected_points.append(closest_point)
        if len(self.selected_points) == 2:
            self.mark_edge()
            self.selected_points = []

    def mark_edge(self):
        if len(self.selected_points) < 2:
            return

        p1, p2 = self.selected_points
        line = self.canvas.create_line(p1['x'], p1['y'], p2['x'], p2['y'], fill='blue')

        self.edges.append((p1, p2, line))

        weight = int(math.sqrt((p1['x'] - p2['x']) ** 2 + (p1['y'] - p2['y']) ** 2))

        self.adjList[self.format_point(p1)].append({
            "destination": {
                "index": p2['index'],
                "x": p2['x'],
                "y": p2['y'],
                "name": p2['name']
            },
            "weight": weight
        })
        self.adjList[self.format_point(p2)].append({
            "destination": {
                "index": p1['index'],
                "x": p1['x'],
                "y": p1['y'],
                "name": p1['name']
            },
            "weight": weight
        })


    def delete_edge(self, event):
        x, y = event.x, event.y
        for p1, p2, line in self.edges:
            x1, y1 = p1['x'], p1['y']
            x2, y2 = p2['x'], p2['y']
            if self.is_near_line(x, y, x1, y1, x2, y2):
                self.canvas.delete(line)
                self.edges.remove((p1, p2, line))
                self.adjList[self.format_point(p1)] = [e for e in self.adjList[self.format_point(p1)] if
                                                       e['destination']['index'] != p2['index']]
                self.adjList[self.format_point(p2)] = [e for e in self.adjList[self.format_point(p2)] if
                                                       e['destination']['index'] != p1['index']]
                break


    def is_near_line(self, x, y, x1, y1, x2, y2, threshold=5):
        """Check if a point (x, y) is near the line segment from (x1, y1) to (x2, y2)."""

        if min(x1, x2) - threshold <= x <= max(x1, x2) + threshold and min(y1, y2) - threshold <= y <= max(y1,y2) + threshold:
            # Calculate distance from point to line segment
            dist = abs((y2 - y1) * x - (x2 - x1) * y + x2 * y1 - y2 * x1) / math.sqrt((y2 - y1) ** 2 + (x2 - x1) ** 2)
            return dist <= threshold
        return False


    def parse_point(self, point_str):
        match = re.match(r"Point\[name=(.*), index=(\d+), x=(\d+), y=(\d+), type=(.*), rating=(\d+\.?\d*), tag=(.*)\]", point_str)
        if match:
            return {
                "name": match.group(1),
                "index": int(match.group(2)),
                "x": int(match.group(3)),
                "y": int(match.group(4)),
                "type": match.group(5),
                "rating": float(match.group(6)),
                "tag": match.group(7)
            }
        return None

    def save_json(self):
        graph = {
            "name": "graph",
            "adjList": self.adjList,
            "shortestDistances": self.calculate_shortest_distances()
        }
        file_path = self.graph_file_path if self.graph_file_path else "graph.json"
        with open(file_path, "w") as f:
            json.dump(graph, f, indent=4)


    def load_graph(self):
        file_path = filedialog.askopenfilename(filetypes=[("JSON files", "*.json")])
        if file_path:
            self.graph_file_path = file_path
            with open(file_path, "r") as f:
                graph = json.load(f)
            self.adjList = graph["adjList"]
            self.points = []
            self.edges = []
            for point_str in self.adjList.keys():
                point = self.parse_point(point_str)
                self.points.append(point)
                self.canvas.create_oval(point['x'] - 3, point['y'] - 3, point['x'] + 3, point['y'] + 3, fill='red')
                self.canvas.create_text(point['x'], point['y'] - 10, text=point['name'], fill='red')
            for point_str, edges in self.adjList.items():
                src_point = self.parse_point(point_str)
                for edge in edges:
                    dest_point = self.points[edge["destination"]["index"]]
                    line = self.canvas.create_line(src_point['x'], src_point['y'], dest_point['x'], dest_point['y'],
                                                   fill='blue')
                    self.edges.append((src_point, dest_point, line))

    def calculate_shortest_distances(self):
        n = len(self.points)
        distances = [[float('inf')] * n for _ in range(n)]
        for i in range(n):
            distances[i][i] = 0

        for src, edges in self.adjList.items():
            src_point = self.parse_point(src)
            if src_point is None:
                continue  # 如果src_point是None，跳过这次循环
            src_index = src_point['index']
            for edge in edges:
                dest_index = edge['destination']['index']
                weight = edge['weight']
                distances[src_index][dest_index] = weight

        for k in range(n):
            for i in range(n):
                for j in range(n):
                    if distances[i][j] > distances[i][k] + distances[k][j]:
                        distances[i][j] = int(distances[i][k] + distances[k][j])

        return distances

    def on_exit(self):
        print(f"Total vertices: {len(self.points)}")
        print(f"Total edges: {len(self.edges)}")


root = tk.Tk()
app = GraphApp(root)
root.mainloop()

