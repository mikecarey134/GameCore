import os

SECOND = 46186158000
HERE = os.path.dirname(os.path.abspath(__file__))
BIND_POSE = {300: (0, 0, 0), 301: (0, 1.5, 0)}
REST_POSE = {300: (0, 0, 0), 301: (0, 1, 0)}

BONES = [
    (300, "Bone1", None, (0, 0, 0)),
    (301, "Bone2", 300, (0, 1, 0)),
    (302, "Bone3", 301, (0, 1, 0)),
]


def numbers(values):
    return ",".join(str(v) for v in values)


def array(name, values):
    return "\t\t%s: *%d {\n\t\t\ta: %s\n\t\t}\n" % (name, len(values), numbers(values))


def translation(offset):
    return [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, offset[0], offset[1], offset[2], 1]


def header():
    return """; FBX 7.4.0 project file
FBXHeaderExtension:  {
\tFBXHeaderVersion: 1003
\tFBXVersion: 7400
\tCreator: "GameCore make_rig_fbx.py"
}
GlobalSettings:  {
\tVersion: 1000
\tProperties70:  {
\t\tP: "UpAxis", "int", "Integer", "",1
\t\tP: "UpAxisSign", "int", "Integer", "",1
\t\tP: "FrontAxis", "int", "Integer", "",2
\t\tP: "FrontAxisSign", "int", "Integer", "",1
\t\tP: "CoordAxis", "int", "Integer", "",0
\t\tP: "CoordAxisSign", "int", "Integer", "",1
\t\tP: "UnitScaleFactor", "double", "Number", "",1
\t}
}
"""


def model(uid, name, kind, offset, rotation=(0, 0, 0)):
    return """\tModel: %d, "Model::%s", "%s" {
\t\tVersion: 232
\t\tProperties70:  {
\t\t\tP: "Lcl Translation", "Lcl Translation", "", "A",%s
\t\t\tP: "Lcl Rotation", "Lcl Rotation", "", "A",%s
\t\t}
\t}
""" % (uid, name, kind, numbers(offset), numbers(rotation))


def geometry(uid, name, vertices, polygons):
    indices = []
    for polygon in polygons:
        indices += polygon[:-1] + [-polygon[-1] - 1]
    return '\tGeometry: %d, "Geometry::%s", "Mesh" {\n' % (uid, name) + array("Vertices", vertices) + \
        array("PolygonVertexIndex", indices) + "\t\tGeometryVersion: 124\n\t}\n"


def clip(uid, name, seconds, curves):
    stop = int(SECOND * seconds)
    objects = """\tAnimationStack: %d, "AnimStack::%s", "" {
\t\tProperties70:  {
\t\t\tP: "LocalStop", "KTime", "Time", "",%d
\t\t\tP: "ReferenceStop", "KTime", "Time", "",%d
\t\t}
\t}
\tAnimationLayer: %d, "AnimLayer::BaseLayer", "" {
\t}
""" % (uid, name, stop, stop, uid + 1)
    connections = '\tC: "OO",%d,%d\n' % (uid + 1, uid)
    for index, (bone, prop, axis, values) in enumerate(curves):
        node = uid + 2 + index * 2
        objects += """\tAnimationCurveNode: %d, "AnimCurveNode::%s", "" {
\t\tProperties70:  {
\t\t\tP: "d|X", "Number", "", "A",0
\t\t\tP: "d|Y", "Number", "", "A",0
\t\t\tP: "d|Z", "Number", "", "A",0
\t\t}
\t}
\tAnimationCurve: %d, "AnimCurve::", "" {
\t\tDefault: 0
\t\tKeyVer: 4009
""" % (node, prop[4], node + 1) + array("KeyTime", [0, stop]) + array("KeyValueFloat", list(values)) + "\t}\n"
        connections += """\tC: "OO",%d,%d
\tC: "OP",%d,%d, "%s"
\tC: "OP",%d,%d, "d|%s"
""" % (node, uid + 1, node, bone, prop, node + 1, node, axis)
    return objects, connections


def skeleton():
    objects = "".join(model(uid, name, "LimbNode", offset) for uid, name, parent, offset in BONES)
    connections = "".join('\tC: "OO",%d,%d\n' % (uid, parent or 0) for uid, name, parent, offset in BONES)
    return objects, connections


def body(bind_pose, parent=0, rotation=(0, 0, 0)):
    corners = [(-0.25, -0.25), (0.25, -0.25), (0.25, 0.25), (-0.25, 0.25)]
    vertices = []
    for level in range(3):
        for x, z in corners:
            vertices += [x, level, z]
    polygons = [[0, 3, 2, 1], [8, 9, 10, 11]]
    for level in range(2):
        for side in range(4):
            a = level * 4 + side
            b = level * 4 + (side + 1) % 4
            polygons.append([a, b, b + 4, a + 4])
    objects = model(200, "Body", "Mesh", (0, 0, 0), rotation) + geometry(100, "Body", vertices, polygons)
    objects += '\tDeformer: 400, "Deformer::Skin", "Skin" {\n\t\tVersion: 101\n\t\tLink_DeformAcuracy: 50\n\t}\n'
    connections = '\tC: "OO",200,%d\n\tC: "OO",100,200\n\tC: "OO",400,100\n' % parent
    for uid, bone, indexes in ((401, 300, list(range(8))), (402, 301, [8, 9, 10, 11])):
        bind = bind_pose[bone]
        objects += '\tDeformer: %d, "SubDeformer::Cluster %d", "Cluster" {\n\t\tVersion: 100\n' % (uid, bone) + \
            array("Indexes", indexes) + array("Weights", [1] * len(indexes)) + \
            array("Transform", translation((0, 0, 0))) + array("TransformLink", translation(bind)) + "\t}\n"
        connections += '\tC: "OO",%d,400\n\tC: "OO",%d,%d\n' % (uid, bone, uid)
    return objects, connections


def base():
    objects = model(210, "Base", "Mesh", (0, -0.5, 0)) + \
        geometry(110, "Base", [-1, 0, -1, 1, 0, -1, 1, 0, 1, -1, 0, 1], [[0, 3, 2, 1]])
    return objects, '\tC: "OO",210,0\n\tC: "OO",110,210\n'


def write(name, parts):
    objects = "".join(part[0] for part in parts)
    connections = "".join(part[1] for part in parts)
    with open(os.path.join(HERE, name), "w", newline="\n") as out:
        out.write(header() + "Objects:  {\n" + objects + "}\nConnections:  {\n" + connections + "}\n")


write("rig.fbx", [skeleton(), body(BIND_POSE), base(),
                  clip(500, "bend", 1.0, [(300, "Lcl Rotation", "X", (0, 45)), (300, "Lcl Translation", "Y", (0, 0.5))]),
                  clip(510, "wave", 1.0, [(301, "Lcl Rotation", "Z", (0, 90))])])
write("static.fbx", [base()])
write("turned.fbx", [skeleton(), (model(220, "Holder", "Null", (3, 0, 0)), '\tC: "OO",220,0\n'),
                     body(REST_POSE, 220, (0, 90, 0))])
write("rig@nod.fbx", [skeleton(), clip(520, "nod", 0.5, [(302, "Lcl Rotation", "X", (0, 30)), (302, "Lcl Scaling", "Y", (2, 2))])])
