from matplotlib import pyplot as plt
import math


LIST_OF_COORDS_PRESENT = (1 << 7)
SHOW_IDS = 0
SCALE = 5
FONT_SIZE = 5

def getLine(file):
    line = file.readline().strip()
    while line.startswith("#"):
        line = file.readline().strip()
    return line

def generateCoords(Slitherlink: dict):
    """
    Generates coordinates for the vertices of the Slitherlink puzzle.

    Args:
        Slitherlink (dict): The Slitherlink puzzle to generate coordinates for.
    """
    size = math.sqrt((len(Slitherlink["vertices"]) / 6))
    assert size == int(size)
    
    for layer_no in range(1, int(size) + 1):
        layer_part_size = 2 * layer_no - 1
        layer_size = 6 * (2 * layer_no - 1)
        vecs = [[0, 0]] * layer_part_size
        vecs[0] = [0, layer_no + (layer_no - 1) // 2]
        
        for i in range(1, layer_no):
            if (layer_no + i) % 2 == 1:
                vecs[i] = [vecs[i - 1][0] + math.sqrt(3) / 2, vecs[i - 1][1] + 0.5]
            else:
                vecs[i] = [vecs[i - 1][0] + math.sqrt(3) / 2, vecs[i - 1][1] - 0.5]

        for i in range(layer_no - 1):
            x = vecs[layer_no - i - 1][0]
            y = vecs[layer_no - i - 1][1]
            r = math.sqrt(x ** 2 + y ** 2)
            angle = -(math.pi / 6 - math.atan(y / x))
            vecs[layer_no + i] = [r * math.sin(angle), r * math.cos(angle)]
        
        for i in range(6):
            for j in range(layer_part_size):
                Slitherlink["vertices"][6 * (layer_no - 1) * (layer_no - 1) + i * layer_part_size + j]["x"] = vecs[j][0]
                Slitherlink["vertices"][6 * (layer_no - 1) * (layer_no - 1) + i * layer_part_size + j]["y"] = vecs[j][1]
            vecs = [[vecs[i][0] * math.cos(-math.pi / 3) - vecs[i][1] * math.sin(-math.pi / 3), vecs[i][0] * math.sin(-math.pi / 3) + vecs[i][1] * math.cos(-math.pi / 3)] for i in range(layer_part_size)]
    
    return Slitherlink


def readPuzzle(file_name: str):
    """
    Reads a puzzle from a file and returns the puzzle as a 2D list.

    Args:
        file_name (str): The name of the file to read the puzzle from.

    Returns:
        
    """
    with open(file_name, "r") as file:
        Slitherlink = {}
        Slitherlink["params_bitmap"] = int(getLine(file), 2)
        Slitherlink["no_of_vertices"] = int(getLine(file))
        Slitherlink["vertices"] = []
        Slitherlink["no_of_edges"] = int(getLine(file))
        Slitherlink["edges"] = []
        Slitherlink["no_of_faces"] = int(getLine(file))
        Slitherlink["faces"] = []
        for _ in range(Slitherlink["no_of_vertices"]):
            vertex = getLine(file).split()
            Slitherlink["vertices"].append({"id": int(vertex[0]), "no_of_edges": int(vertex[1]), "edge_ids": [int(val) for val in vertex[2:]]})
        
        for _ in range(Slitherlink["no_of_edges"]):
            edge = getLine(file).split()
            Slitherlink["edges"].append({"id": int(edge[0]), "vertex_id_1": int(edge[1]), "vertex_id_2": int(edge[2]), "face_id_1": int(edge[3]), "face_id_2": int(edge[4]), "solution": int(edge[5])})

        for _ in range(Slitherlink["no_of_faces"]):
            face = getLine(file).split()
            Slitherlink["faces"].append({"id": int(face[0]), "value": int(face[1]), "no_of_edges": int(face[2]), "edge_ids": [int(val) for val in face[3:]]})
    
        if Slitherlink["params_bitmap"] & LIST_OF_COORDS_PRESENT:
            for _ in range(Slitherlink["no_of_vertices"]):
                coords = getLine(file).split()
                Slitherlink["vertices"][int(coords[0])]["x"] = int(coords[1])
                Slitherlink["vertices"][int(coords[0])]["y"] = int(coords[2])
        else:
            Slitherlink = generateCoords(Slitherlink)
        for i in range(Slitherlink["no_of_vertices"]):
            Slitherlink["vertices"][i]["x"] *= SCALE
            Slitherlink["vertices"][i]["y"] *= SCALE
    return Slitherlink 

def plotPuzzle(Slitherlink: dict):
    """
    Plots a Slitherlink puzzle.

    Args:
        Slitherlink (dict): The Slitherlink puzzle to plot.
    """
    fig, ax = plt.subplots()
    for edge in Slitherlink["edges"]:
        vertex_1 = Slitherlink["vertices"][edge["vertex_id_1"]]
        vertex_2 = Slitherlink["vertices"][edge["vertex_id_2"]]
        if edge["solution"] == 1:
            style = "-"
            col = "green"
        else:
            style = ":"
            col = "grey"
        ax.plot([vertex_1["x"], vertex_2["x"]], [vertex_1["y"], vertex_2["y"]], style, color=col)
        if SHOW_IDS:
            ax.text((vertex_1["x"] + vertex_2["x"]) / 2, (vertex_1["y"] + vertex_2["y"]) / 2, edge["id"], fontsize=FONT_SIZE, color="blue")
    for vertex in Slitherlink["vertices"]:
        ax.plot(vertex["x"], vertex["y"], "o", color="black", ms=2)
        if SHOW_IDS:
            ax.text(vertex["x"], vertex["y"], vertex["id"], fontsize=FONT_SIZE)
    if SHOW_IDS:
        for face in Slitherlink["faces"]:
            if face["value"] != -1:
                x_vec = [(Slitherlink["vertices"][Slitherlink["edges"][edge_id]["vertex_id_1"]]["x"] + Slitherlink["vertices"][Slitherlink["edges"][edge_id]["vertex_id_2"]]["x"])/2 for edge_id in face["edge_ids"]]
                y_vec = [(Slitherlink["vertices"][Slitherlink["edges"][edge_id]["vertex_id_1"]]["y"] + Slitherlink["vertices"][Slitherlink["edges"][edge_id]["vertex_id_2"]]["y"])/2 for edge_id in face["edge_ids"]]
                ax.text(sum(x_vec)/6, sum(y_vec)/6, face["id"], fontsize=FONT_SIZE, color="red")
    ax.set_aspect("equal")
    plt.axis("off")
    plt.savefig("test.png", dpi=1200)

plotPuzzle(readPuzzle("generated_1.txt"))
