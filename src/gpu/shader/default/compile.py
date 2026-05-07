import subprocess

common_tasks = [
    {"src": "shader.slang", "entry": "FSMain", "out": "shader.frag"},
    {"src": "font.slang", "entry": "FSMain", "out": "font.frag"},
    {"src": "shader.slang", "entry": "VSMain", "out": "shader.vert"},
    {
        "src": "shader_instance.slang",
        "entry": "VSMain",
        "out": "shader_instance.vert",
    },
    {"src": "rect_color.slang", "entry": "FSMain", "out": "rect_color.frag"},
    {"src": "cubemap.slang", "entry": "FSMain", "out": "cubemap.frag"},
    {"src": "cubemap.slang", "entry": "VSMain", "out": "cubemap.vert"},
]

spirv_tasks = [{**task, "out": f"{task['out']}.spv"} for task in common_tasks]
wgsl_tasks = [{**task, "out": f"{task['out']}.wgsl"} for task in common_tasks]


def generate_headers(tasks):
    for task in tasks:
        shader_file = task["out"]
        header_file = f"{shader_file}.hpp"
        cmd = ["xxd", "-i", shader_file]
        print("Generating header:", header_file)
        with open(header_file, "w") as f:
            subprocess.run(cmd, stdout=f, check=True)


for task in spirv_tasks:
    cmd = [
        "slangc",
        "-profile",
        "glsl_450",
        "-target",
        "spirv",
        "-matrix-layout-row-major",
        "-entry",
        task["entry"],
        task["src"],
        "-o",
        task["out"],
    ]
    print("Running:", " ".join(cmd))
    subprocess.run(cmd, check=True)

generate_headers(spirv_tasks)

for task in wgsl_tasks:
    cmd = [
        "slangc",
        "-target",
        "wgsl",
        "-matrix-layout-row-major",
        "-entry",
        task["entry"],
        task["src"],
        "-o",
        task["out"],
    ]
    print("Running:", " ".join(cmd))
    subprocess.run(cmd, check=True)

generate_headers(wgsl_tasks)


dxil_tasks = [
    {"src": "shader.slang", "entry": "FSMain", "out": "shader.frag.dxil", "profile": "ps_6_0"},
    {"src": "font.slang", "entry": "FSMain", "out": "font.frag.dxil", "profile": "ps_6_0"},
    {"src": "shader.slang", "entry": "VSMain", "out": "shader.vert.dxil", "profile": "vs_6_0"},
    {
        "src": "shader_instance.slang",
        "entry": "VSMain",
        "out": "shader_instance.vert.dxil",
        "profile": "vs_6_0",
    },
    {"src": "rect_color.slang", "entry": "FSMain", "out": "rect_color.frag.dxil", "profile": "ps_6_0"},
    {"src": "cubemap.slang", "entry": "FSMain", "out": "cubemap.frag.dxil", "profile": "ps_6_0"},
    {"src": "cubemap.slang", "entry": "VSMain", "out": "cubemap.vert.dxil", "profile": "vs_6_0"},
]
for task in dxil_tasks:
    cmd = [
        "slangc",
        "-profile",
        task["profile"],
        "-target",
        "dxil",
        "-matrix-layout-row-major",
        "-entry",
        task["entry"],
        task["src"],
        "-o",
        task["out"],
    ]
    print("Running:", " ".join(cmd))
    subprocess.run(cmd, check=True)

for task in dxil_tasks:
    shader_file = task["out"]
    header_file = f"{shader_file}.hpp"
    cmd = ["xxd", "-i", shader_file]
    print("Generating header:", header_file)
    with open(header_file, "w") as f:
        subprocess.run(cmd, stdout=f, check=True)
