import subprocess

# 1. Shader info
shader_tasks = [
    {
        "src": "shader.slang",
        "entry": "FSMain",
        "out": "shader.frag.spv"
    },
    {
        "src": "shader.slang",
        "entry": "VSMain",
        "out": "shader.vert.spv"
    },
    {
        "src": "shader_instance.slang",
        "entry": "VSMain",
        "out": "shader_instance.vert.spv"
    }
]

# 2. SPIR-V gen using slangc
for task in shader_tasks:
    cmd = [
        "slangc",
        "-profile", "glsl_450",
        "-target", "spirv",
        "-entry", task["entry"],
        task["src"],
        "-o", task["out"]
    ]
    print("Running:", " ".join(cmd))
    subprocess.run(cmd, check=True)

# 3. Header File using xxd
for task in shader_tasks:
    spv_file = task["out"]
    header_file = f"{spv_file}.h"
    cmd = [
        "xxd", "-i", spv_file
    ]
    print("Generating header:", header_file)
    with open(header_file, "w") as f:
        subprocess.run(cmd, stdout=f, check=True)

print("All complete!")

