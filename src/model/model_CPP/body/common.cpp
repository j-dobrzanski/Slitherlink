#include "../api/common.hpp"


std::ptrdiff_t getNoOfVertices(std::ptrdiff_t layer_no) {
    return 6 * layer_no * layer_no;
}

std::ptrdiff_t getNoOfEdges(std::ptrdiff_t layer_no) {
    return 9 * layer_no * layer_no - 3 * layer_no;
}

std::ptrdiff_t getNoOfFaces(std::ptrdiff_t layer_no) {
    return 3 * layer_no * layer_no - 3 * layer_no + 1;
}

std::ostream & operator<<(std::ostream& os, const slitherlink_vertex& vertex) {
    os << "Vertex\n{\n    id:" << vertex.id << "\n";
    os << "    no_of_edges: " << vertex.no_of_edges << "\n";
    os << "    edges: ";
    for (const auto& edge_id : vertex.edge_ids) {
        os << edge_id << " ";
    }
    os << "\n}\n";
    return os;
}

std::ostream & operator<<(std::ostream& os, const slitherlink_edge& edge) {
    os << "Edge\n{\n    id:" << edge.id << "\n";
    os << "    vertices: ";
    for (const auto& vertex : edge.vertices) {
        os << vertex << " ";
    }
    os << "\n    faces: ";
    for (const auto& face_id : edge.face_ids) {
        os << face_id << " ";
    }
    os << "\n    solution: " << edge.solution << "\n";
    os << "\n}\n";
    return os;
}

std::ostream & operator<<(std::ostream& os, const slitherlink_face& face) {
    os << "Face\n{\n    id:" << face.id << "\n";
    os << "    value: " << face.value << "\n";
    os << "    no_of_edges: " << face.no_of_edges << "\n";
    os << "    edges: ";
    for (const auto& edge_id : face.edge_ids) {
        os << edge_id << " ";
    }
    os << "\n}\n";
    return os;
}