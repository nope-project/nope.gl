import array
import math

from pynopegl_utils.misc import SceneCfg, scene

import pynopegl as ngl


@scene(square_color=scene.Color(), circle_color=scene.Color())
def square2circle(cfg: SceneCfg, square_color=(0.9, 0.1, 0.3), circle_color=(1.0, 1.0, 1.0)):
    """Morphing of a square (composed of many vertices) into a circle"""
    cfg.duration = 5
    cfg.aspect_ratio = (1, 1)

    def sqxf(t):  # square x coordinates clockwise starting top-left
        if t < 1 / 4.0:
            return t * 4
        if t < 1 / 2.0:
            return 1
        if t < 3 / 4.0:
            return 1.0 - (t - 0.5) * 4
        return 0

    def sqyf(t):  # square y coordinates clockwise starting top-left
        if t < 1 / 4.0:
            return 1
        if t < 1 / 2.0:
            return 1.0 - (t - 0.25) * 4
        if t < 3 / 4.0:
            return 0
        return (t - 0.75) * 4

    n = 1024  # number of vertices
    s = 1.25  # shapes scale
    interp = "exp_in_out"

    center_vertex = [0, 0, 0]
    square_vertices = array.array("f", center_vertex)
    for i in range(n):
        x = (sqxf(i / float(n)) - 0.5) * s
        y = (sqyf(i / float(n)) - 0.5) * s
        square_vertices.extend([x, y, 0])

    circle_vertices = array.array("f", center_vertex)
    step = 2 * math.pi / float(n)
    for i in range(n):
        angle = i * step - math.pi / 4.0
        x = math.sin(angle) * 0.5 * s
        y = math.cos(angle) * 0.5 * s
        circle_vertices.extend([x, y, 0])

    indices = array.array("H")
    for i in range(1, n + 1):
        indices.extend([0, i, i + 1])
    indices[-1] = 1

    vertices_animkf = [
        ngl.AnimKeyFrameBuffer(0, square_vertices),
        ngl.AnimKeyFrameBuffer(cfg.duration / 2.0, circle_vertices, interp),
        ngl.AnimKeyFrameBuffer(cfg.duration, square_vertices, interp),
    ]
    vertices = ngl.AnimatedBufferVec3(vertices_animkf)

    color_animkf = [
        ngl.AnimKeyFrameColor(0, square_color),
        ngl.AnimKeyFrameColor(cfg.duration / 2.0, circle_color, interp),
        ngl.AnimKeyFrameColor(cfg.duration, square_color, interp),
    ]
    ucolor = ngl.AnimatedColor(color_animkf)

    geom = ngl.Geometry(vertices, indices=ngl.BufferUShort(data=indices))
    p = ngl.Program(vertex=cfg.get_vert("color"), fragment=cfg.get_frag("color"))
    render = ngl.Render(geom, p)
    render.update_frag_resources(color=ucolor, opacity=ngl.UniformFloat(1))
    return render


@scene(npoints=scene.Range(range=[3, 100]))
def urchin(cfg: SceneCfg, npoints=25):
    """Urchin with animated vertices"""
    cfg.duration = 5
    cfg.aspect_ratio = (1, 1)

    def get_vertices(n, radius_func, offset=0.0):
        vertices = []
        step = 2 * math.pi / n
        for i in range(n):
            angle = (i + offset) * step
            radius = radius_func()
            x, y = math.sin(angle) * radius, math.cos(angle) * radius
            vertices.append([x, y, 0])
        return vertices

    k = 16
    n, m = 0.1, 0.9

    inner_rfunc = lambda: n
    inner_vertices = get_vertices(npoints, inner_rfunc)

    vdata = []
    for i in range(k):
        outer_rfunc = lambda: cfg.rng.uniform(n, m)
        outer_vertices = get_vertices(npoints, outer_rfunc, offset=0.5)
        vertices_data = array.array("f")
        for inner_vertex, outer_vertex in zip(inner_vertices, outer_vertices):
            vertices_data.extend(inner_vertex + outer_vertex)
        vertices_data.extend(inner_vertices[0])
        vdata.append(vertices_data)

    animkf = []
    for i, v in enumerate(vdata + [vdata[0]]):
        animkf.append(ngl.AnimKeyFrameBuffer(i * cfg.duration / float(k), v))

    vertices = ngl.AnimatedBufferVec3(animkf)

    geom = ngl.Geometry(vertices)
    geom.set_topology("line_strip")
    p = ngl.Program(vertex=cfg.get_vert("color"), fragment=cfg.get_frag("color"))
    render = ngl.Render(geom, p)
    render.update_frag_resources(color=ngl.UniformVec3(value=(0.9, 0.1, 0.3)), opacity=ngl.UniformFloat(1))
    return render
