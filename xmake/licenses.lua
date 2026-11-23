-- Generate the license report from the pinned sources.
local entries = {
	{"Sinen", "https://github.com/astomih/sinen", "LICENSE.md", 0, ""},
	{"SDL", "https://github.com/libsdl-org/SDL", "libs/SDL/LICENSE.txt", 0, ""},
	{"Assimp", "https://github.com/assimp/assimp", "libs/assimp/LICENSE", 0, ""},
	{"Box2D", "https://github.com/erincatto/box2d", "libs/box2d/LICENSE", 0, ""},
	{"Box3D", "https://github.com/erincatto/box3d", "libs/box3d/LICENSE", 0, ""},
	{"cppdap", "https://github.com/google/cppdap", "libs/cppdap/LICENSE", 0, ""},
	{"KTX-Software", "https://github.com/KhronosGroup/KTX-Software", "libs/ktx-software/LICENSE.md", 0, ""},
	{"KTX-Software NOTICE", "https://github.com/KhronosGroup/KTX-Software", "libs/ktx-software/NOTICE.md", 0, ""},
	{"Luau", "https://github.com/luau-lang/luau", "libs/luau/LICENSE.txt", 0, ""},
	{"Luau Debugger", "https://github.com/sssooonnnggg/luau-debugger", "", 0, "No license file is distributed in the pinned source revision."},
	{"meshoptimizer", "https://github.com/zeux/meshoptimizer", "libs/meshoptimizer/LICENSE.md", 0, ""},
	{"miniaudio", "https://github.com/mackron/miniaudio", "libs/miniaudio/LICENSE", 0, ""},
	{"OpenH264", "https://github.com/cisco/openh264", "libs/openh264/LICENSE", 0, ""},
	{"minimp4", "https://github.com/lieff/minimp4", "libs/minimp4/LICENSE", 0, ""},
	{"msdfgen", "https://github.com/Chlumsky/msdfgen", "libs/msdfgen/LICENSE.txt", 0, ""},
	{"RapidJSON", "https://github.com/Tencent/rapidjson", "libs/rapidjson/license.txt", 0, ""},
	{"RenderDoc API", "https://github.com/baldurk/renderdoc", "libs/renderdoc/renderdoc_app.h", 23, "MIT license for the in-application API header."},
	{"Slang", "https://github.com/shader-slang/slang", "plugins/shader_compiler/libs/slang/LICENSE", 0, ""},
	{"stb", "https://github.com/nothings/stb", "libs/stb/LICENSE", 0, ""},
	{"TinyEXR", "https://github.com/syoyo/tinyexr", "libs/tinyexr/tinyexr.h", 68, "License notices extracted from the beginning of tinyexr.h."},
	{"TLSF", "https://github.com/mattconte/tlsf", "libs/tlsf/tlsf.h", 40, "License notice extracted from the beginning of tlsf.h."},
	{"Tracy", "https://github.com/wolfpld/tracy", "libs/tracy/LICENSE", 0, ""},
	{"volk", "https://github.com/zeux/volk", "libs/volk/LICENSE.md", 0, ""},
	{"Vulkan Memory Allocator", "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator", "libs/VulkanMemoryAllocator/LICENSE.txt", 0, ""},
	{"WebGPU-distribution", "https://github.com/eliemichel/WebGPU-distribution", "libs/WebGPU-distribution/LICENSE.txt", 0, ""},
}

function main(output)
	local root = os.projectdir()
	local lines = {}
	for _, entry in ipairs(entries) do
		local name, url, file, max_lines, note = table.unpack(entry)
		local input = path.join(root, file)
		local license = ""
		if file ~= "" and os.isfile(input) then
			license = io.readfile(input, {encoding = "binary"})
			if max_lines > 0 then
				local excerpt = {}
				for line in license:gmatch("[^\r\n]+") do
					table.insert(excerpt, line)
					if #excerpt == max_lines then break end
				end
				license = table.concat(excerpt, "\n") .. "\n"
			end
		end
		local hex = license:gsub(".", function (char) return string.format("%02x", char:byte()) end)
		local chunks = {}
		for index = 0, 3 do
			table.insert(chunks, string.format("%q", hex:sub(index * 8000 + 1, (index + 1) * 8000)))
		end
		table.insert(lines, string.format("    {%q, %q, %q, %q, {%s}},", name, url, file, note, table.concat(chunks, ", ")))
	end
	local content = [[#pragma once

#include <array>
#include <string_view>

namespace sinen::license_data {
struct EmbeddedLicense {
  std::string_view name;
  std::string_view url;
  std::string_view source;
  std::string_view note;
  std::array<std::string_view, 4> textHexChunks;
};

inline constexpr EmbeddedLicense entries[] = {
]] .. table.concat(lines, "\n") .. "\n};\n} // namespace sinen::license_data\n"
	if not os.isfile(output) or io.readfile(output) ~= content then
		os.mkdir(path.directory(output))
		io.writefile(output, content)
	end
end
