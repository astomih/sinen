--!strict

-- This file is a Luau definition file for Sinen's Lua API.

export type Buffer = {
	new: (array: { any }) -> Buffer,
	fromBytes: (bytes: { number }) -> Buffer,
	size: (self: Buffer) -> number,
	toHeader: (self: Buffer, symbolName: string) -> Buffer,
}

local Vec3Impl = {}
Vec3Impl.new = function(): Vec3
	return setmetatable(
		{} :: {
			x: number,
			y: number,
			z: number,
			new: ((x: number, y: number, z: number) -> Vec3) & ((value: number) -> Vec3),
			copy: (self: Vec3) -> Vec3,
			length: (self: Vec3) -> number,
			forward: (self: Vec3, rotation: Vec3) -> Vec3,
			normalize: (self: Vec3) -> Vec3,
			dot: (self: Vec3, other: Vec3) -> number,
			cross: (self: Vec3, other: Vec3) -> Vec3,
			lerp: (self: Vec3, other: Vec3, t: number) -> Vec3,
			reflect: (self: Vec3, n: Vec3) -> Vec3,
		},
		{} :: typeof(Vec3Impl)
	)
end
Vec3Impl.__index = Vec3Impl
Vec3Impl.__add = function(a: Vec3, b: Vec3): Vec3
	error("Not implemented")
end
Vec3Impl.__mul = function(a: Vec3, b: Vec3 | number): Vec3
	error("Not implemented")
end
Vec3Impl.__sub = function(a: Vec3, b: Vec3): Vec3
	error("Not implemented")
end
Vec3Impl.__div = function(a: Vec3, b: Vec3 | number): Vec3
	error("Not implemented")
end
export type Vec3 = typeof(Vec3Impl.new())

local Vec2Impl = {}
Vec2Impl.new = function(): Vec2
	return setmetatable(
		{} :: {
			x: number,
			y: number,
			new: ((x: number, y: number) -> Vec2) & ((value: number) -> Vec2),
			copy: (self: Vec2) -> Vec2,
			length: (self: Vec2) -> number,
			normalize: (self: Vec2) -> Vec2,
			dot: (self: Vec2, other: Vec2) -> number,
			lerp: (self: Vec2, other: Vec2, t: number) -> Vec2,
			reflect: (self: Vec2, n: Vec2) -> Vec2,
		},
		Vec2Impl
	)
end

Vec2Impl.__index = Vec2Impl
Vec2Impl.__add = function(a: Vec2, b: Vec2): Vec2
	error("Not implemented")
end
Vec2Impl.__mul = function(a: Vec2, b: Vec2 | number): Vec2
	error("Not implemented")
end
Vec2Impl.__sub = function(a: Vec2, b: Vec2): Vec2
	error("Not implemented")
end
Vec2Impl.__div = function(a: Vec2, b: Vec2 | number): Vec2
	error("Not implemented")
end
export type Vec2 = typeof(Vec2Impl.new())

export type Pivot = {
	TopLeft: Pivot,
	TopCenter: Pivot,
	TopRight: Pivot,
	Left: Pivot,
	Center: Pivot,
	Right: Pivot,
	BottomLeft: Pivot,
	BottomCenter: Pivot,
	BottomRight: Pivot,
}

-- 2D rectangles use a top-left origin: x/y is the top-left corner,
-- width grows to the right, and height grows downward.
-- The pivot overloads let you construct the same rectangle from another anchor.
export type Rect = {
	x: number,
	y: number,
	width: number,
	height: number,

	new: (
		((x: number, y: number, width: number, height: number) -> Rect)
		& ((position: Vec2, size: Vec2) -> Rect)
		& ((pivot: Pivot, x: number, y: number, w: number, h: number) -> Rect)
		& ((pivot: Pivot, pos: Vec2, size: Vec2) -> Rect)
	),
	topLeft: (self: Rect) -> Vec2,
	topCenter: (self: Rect) -> Vec2,
	topRight: (self: Rect) -> Vec2,
	left: (self: Rect) -> Vec2,
	center: (self: Rect) -> Vec2,
	right: (self: Rect) -> Vec2,
	bottomLeft: (self: Rect) -> Vec2,
	bottomCenter: (self: Rect) -> Vec2,
	bottomRight: (self: Rect) -> Vec2,
	position: (self: Rect) -> Vec2,
	positionFromPivot: (self: Rect, pivot: Pivot) -> Vec2,
	size: (self: Rect) -> Vec2,
	intersectsRect: (self: Rect, rect: Rect) -> boolean,
}

export type Transform = {
	new: () -> Transform,
	position: Vec3,
	rotation: Vec3,
	scale: Vec3,
}

export type AABB = {
	new: () -> AABB,
	min: Vec3,
	max: Vec3,
	updateWorld: (self: AABB, position: Vec3, scale: Vec3, modelAABB: AABB) -> nil,
	intersectsAABB: (self: AABB, aabb: AABB) -> boolean,
}

export type Color = {
	new: ((r: number, g: number, b: number, a: number) -> Color)
		& ((value: number, a: number) -> Color)
		& ((value: number) -> Color)
		& (() -> Color),
	r: number,
	g: number,
	b: number,
	a: number,
}

export type Texture = {
	new: (() -> Texture) & ((width: number, height: number) -> Texture),
	fill: (self: Texture, color: Color) -> nil,
	blend: (self: Texture, color: Color) -> nil,
	copy: (self: Texture) -> Texture,
	load: (self: Texture, path: string) -> nil,
	loadPixels: (self: Texture, pixels: Buffer, width: number, height: number, format: string?) -> boolean,
	loadCubemap: (self: Texture, path: string) -> nil,
	loadIrradianceCubemap: (self: Texture, path: string, faceSize: number?, sampleCount: number?) -> boolean,
	loadPrefilteredCubemap: (
		self: Texture,
		path: string,
		faceSize: number?,
		mipLevels: number?,
		sampleCount: number?
	) -> boolean,
	loadBRDFLUT: (self: Texture, size: number?, sampleCount: number?) -> boolean,
	loadFromPath: (self: Texture, path: string) -> nil,
	size: (self: Texture) -> Vec2,
}

export type RenderTexture = {
	new: () -> RenderTexture,
	create: (self: RenderTexture, x: number, y: number) -> nil,
}

export type Sound = {
	new: () -> Sound,
	load: (self: Sound, path: string) -> nil,
	loadFromPath: (self: Sound, path: string) -> nil,
	play: (self: Sound) -> nil,
	setVolume: (self: Sound, volume: number) -> nil,
	setPitch: (self: Sound, pitch: number) -> nil,
	setListener: (self: Sound, position: Vec3) -> nil,
	setPosition: (self: Sound, position: Vec3) -> nil,
}

export type Camera3D = {
	new: () -> Camera3D,
	lookat: (self: Camera3D, position: Vec3, target: Vec3, up: Vec3) -> nil,
	perspective: (self: Camera3D, fov: number, aspect: number, near: number, far: number) -> nil,
	orthographic: (self: Camera3D, width: number, height: number, near: number, far: number) -> nil,
	getPosition: (self: Camera3D) -> Vec3,
	getTarget: (self: Camera3D) -> Vec3,
	getUp: (self: Camera3D) -> Vec3,
	isAABBInFrustum: (self: Camera3D, aabb: AABB) -> boolean,
}

export type Camera2D = {
	new: () -> Camera2D,
	resize: (self: Camera2D, size: Vec2) -> nil,
	windowRatio: (self: Camera2D) -> Vec2,
	invWindowRatio: (self: Camera2D) -> Vec2,
	size: (self: Camera2D) -> Vec2,
	half: (self: Camera2D) -> Vec2,
	rect: (self: Camera2D) -> Rect,
	topLeft: (self: Camera2D) -> Vec2,
	topCenter: (self: Camera2D) -> Vec2,
	topRight: (self: Camera2D) -> Vec2,
	left: (self: Camera2D) -> Vec2,
	center: (self: Camera2D) -> Vec2,
	right: (self: Camera2D) -> Vec2,
	bottomLeft: (self: Camera2D) -> Vec2,
	bottomCenter: (self: Camera2D) -> Vec2,
	bottomRight: (self: Camera2D) -> Vec2,
}

export type TextureKey = {
	BaseColor: number,
	Normal: number,
	DiffuseRoughness: number,
	Metalness: number,
	Emissive: number,
	LightMap: number,
}

export type Model = {
	new: () -> Model,
	getAABB: (self: Model) -> AABB,
	load: ((self: Model, path: string) -> nil) & ((self: Model, buffer: Buffer) -> nil),
	loadSprite: (self: Model) -> nil,
	loadBox: (self: Model) -> nil,
	getBoneUniformBuffer: (self: Model) -> Buffer,
	play: (self: Model, position: number) -> nil,
	update: (self: Model, delta: number) -> nil,
	hasTexture: (self: Model, textureKey: number) -> boolean,
	getTexture: (self: Model, textureKey: number) -> Texture,
	setTexture: (self: Model, textureKey: number, texture: Texture) -> nil,
}

export type MeshBuilder = {
	new: () -> MeshBuilder,
	clear: (self: MeshBuilder) -> nil,
	vertex: (self: MeshBuilder, position: Vec3, normal: Vec3?, uv: Vec2?, color: Color?) -> number,
	triangle: (self: MeshBuilder, a: number, b: number, c: number) -> nil,
	quad: (self: MeshBuilder, a: number, b: number, c: number, d: number) -> nil,
	addPlane: (self: MeshBuilder, width: number?, depth: number?, color: Color?) -> nil,
	addBox: (
		((self: MeshBuilder, size: Vec3?, color: Color?) -> nil)
		& ((self: MeshBuilder, size: number, color: Color?) -> nil)
		& ((self: MeshBuilder, width: number, height: number, depth: number, color: Color?) -> nil)
	),
	addSphere: (self: MeshBuilder, radius: number?, rings: number?, segments: number?, color: Color?) -> nil,
	addCylinder: (self: MeshBuilder, radius: number?, height: number?, segments: number?, color: Color?) -> nil,
	addCone: (self: MeshBuilder, radius: number?, height: number?, segments: number?, color: Color?) -> nil,
	recalculateNormals: (self: MeshBuilder) -> nil,
	toModel: (self: MeshBuilder) -> Model,
	vertexCount: (self: MeshBuilder) -> number,
	indexCount: (self: MeshBuilder) -> number,
}

export type Timer = {
	new: () -> Timer,
	start: (self: Timer) -> nil,
	stop: (self: Timer) -> nil,
	isStarted: (self: Timer) -> boolean,
	setTime: (self: Timer, time: number) -> nil,
	check: (self: Timer) -> boolean,
}

export type ShaderStage = {
	Vertex: ShaderStage,
	Fragment: ShaderStage,
	Compute: ShaderStage,
	RayGeneration: ShaderStage,
	AnyHit: ShaderStage,
	ClosestHit: ShaderStage,
	Miss: ShaderStage,
	Intersection: ShaderStage,
	Callable: ShaderStage,
}

export type ShaderFormat = {
	SPIRV: ShaderFormat,
	WGSL: ShaderFormat,
	DXBC: ShaderFormat,
	DXIL: ShaderFormat,
}

--- Shader resource binding conventions:
---
--- SPIR-V:
---   Vertex:   set 0 = sampled textures, then storage textures, then storage buffers; set 1 = uniform buffers
---   Fragment: set 2 = sampled textures, then storage textures, then storage buffers; set 3 = uniform buffers
---   Compute:  set 0 = storage buffers; set 1 = uniform buffers
---   Ray tracing pipeline: set 4 = acceleration structures, then storage buffers; set 5 = uniform buffers
---   Ray query in graphics/compute shaders: set 6 = acceleration structures
---
--- DXBC / DXIL:
---   Vertex:   t[n]/space0 = sampled textures, then storage textures, then storage buffers;
---             s[n]/space0 = samplers matching sampled-texture indices; b[n]/space1 = uniform buffers
---   Pixel:    t[n]/space2 = sampled textures, then storage textures, then storage buffers;
---             s[n]/space2 = samplers matching sampled-texture indices; b[n]/space3 = uniform buffers
---   Compute:  u[n]/space0 = storage buffers; b[n]/space0 = uniform buffers
---   Ray tracing pipeline: t[n]/space4 = acceleration structures; u[n]/space4 = storage buffers; b[n]/space5 = uniform buffers
---   Ray query in graphics/compute shaders: t[n]/space6 = acceleration structures
export type Shader = {
	new: () -> Shader,
	load: (self: Shader, path: string, shaderStage: ShaderStage, uniform_count: number) -> nil,
	compile: (self: Shader, source: string, shaderStage: ShaderStage, shaderFormat: ShaderFormat) -> nil,
	compileAndLoad: (self: Shader, source: string, shaderStage: ShaderStage, shaderFormat: ShaderFormat?) -> nil,
	getCode: (self: Shader) -> Buffer,
	isReady: (self: Shader) -> boolean,
}

export type ShaderBundleEntry = {
	stage: ShaderStage,
	format: ShaderFormat,
	code: Buffer,
	numSamplers: number?,
	numStorageBuffers: number?,
	numStorageTextures: number?,
	numUniformBuffers: number?,
}

export type ShaderBundle = {
	pack: (entries: { ShaderBundleEntry }) -> Buffer,
}

export type Font = {
	Bitmap: number,
	MSDF: number,
	new: (() -> Font)
		& ((size: number) -> Font)
		& ((size: number, path: string) -> Font)
		& ((size: number, buffer: Buffer) -> Font),
	load: (
		((self: Font, size: number) -> nil)
		& ((self: Font, size: number, method: number) -> nil)
		& ((self: Font, size: number, path: string) -> nil)
		& ((self: Font, size: number, path: string, method: number) -> nil)
		& ((self: Font, size: number, buffer: Buffer) -> nil)
		& ((self: Font, size: number, buffer: Buffer, method: number) -> nil)
	),
	resize: (self: Font, size: number) -> nil,
	region: (self: Font, text: string, fontSize: number, pivot: Pivot, position: Vec2) -> Rect,
}

export type GraphicsPipeline = {
	new: () -> GraphicsPipeline,
	setVertexShader: (self: GraphicsPipeline, shader: Shader) -> nil,
	setFragmentShader: (self: GraphicsPipeline, shader: Shader) -> nil,
	setEnableDepthTest: (self: GraphicsPipeline, enable: boolean) -> nil,
	setEnableInstanced: (self: GraphicsPipeline, enable: boolean) -> nil,
	setEnableAnimation: (self: GraphicsPipeline, enable: boolean) -> nil,
	setEnableTangent: (self: GraphicsPipeline, enable: boolean) -> nil,
	build: (self: GraphicsPipeline) -> nil,
}

export type ComputeBuffer = {
	new: (() -> ComputeBuffer) & ((size: number) -> ComputeBuffer) & ((data: Buffer) -> ComputeBuffer),
	create: (self: ComputeBuffer, size: number) -> nil,
	upload: (self: ComputeBuffer, data: Buffer) -> nil,
	download: (self: ComputeBuffer) -> Buffer,
	size: (self: ComputeBuffer) -> number,
}

export type ComputePipeline = {
	new: () -> ComputePipeline,
	setShader: (self: ComputePipeline, shader: Shader) -> nil,
	setThreadGroupSize: (self: ComputePipeline, x: number, y: number?, z: number?) -> nil,
	build: (self: ComputePipeline) -> nil,
}

export type RaytracingAccelerationStructure = {
	getDeviceAddress: (self: RaytracingAccelerationStructure) -> number,
}

export type RaytracingPipeline = {
	new: () -> RaytracingPipeline,
	addShader: (self: RaytracingPipeline, shader: Shader, exportName: string?) -> nil,
	addHitGroup: (
		self: RaytracingPipeline,
		exportName: string,
		closestHitShaderIndex: number?,
		anyHitShaderIndex: number?,
		intersectionShaderIndex: number?
	) -> nil,
	setMaxPayloadSize: (self: RaytracingPipeline, size: number) -> nil,
	setMaxAttributeSize: (self: RaytracingPipeline, size: number) -> nil,
	setMaxRecursionDepth: (self: RaytracingPipeline, depth: number) -> nil,
	build: (self: RaytracingPipeline) -> nil,
	isReady: (self: RaytracingPipeline) -> boolean,
	getShaderGroupHandleSize: (self: RaytracingPipeline) -> number,
	getShaderGroupHandles: (self: RaytracingPipeline, firstGroup: number, groupCount: number) -> Buffer,
}

export type RaytracingTriangleGeometry = {
	vertexBuffer: ComputeBuffer,
	vertexOffset: number?,
	vertexStride: number?,
	vertexCount: number,
	indexBuffer: ComputeBuffer?,
	indexOffset: number?,
	indexCount: number?,
	opaque: boolean?,
}

export type RaytracingAabbGeometry = {
	aabbBuffer: ComputeBuffer,
	offset: number?,
	stride: number?,
	count: number,
	opaque: boolean?,
}

export type RaytracingInstance = {
	accelerationStructure: RaytracingAccelerationStructure,
	transform: { number }?,
	instanceCustomIndex: number?,
	mask: number?,
	shaderBindingTableRecordOffset: number?,
	flags: number?,
}

export type RaytracingShaderTableRegion = {
	offset: number?,
	size: number?,
	stride: number?,
}

export type RaytracingDispatchInfo = {
	shaderTable: ComputeBuffer,
	width: number,
	height: number?,
	depth: number?,
	rayGeneration: RaytracingShaderTableRegion?,
	miss: RaytracingShaderTableRegion?,
	hit: RaytracingShaderTableRegion?,
	callable: RaytracingShaderTableRegion?,
	accelerationStructures: { RaytracingAccelerationStructure }?,
	storageBuffers: { ComputeBuffer }?,
	uniforms: { Buffer }?,
}

export type RaytracingBuildFlags = {
	None: number,
	AllowUpdate: number,
	PreferFastTrace: number,
	PreferFastBuild: number,
	MinimizeMemory: number,
}

export type Raytracing = {
	Pipeline: RaytracingPipeline,
	BuildFlags: RaytracingBuildFlags,
	isDeviceSupported: () -> boolean,
	isSupported: () -> boolean,
	isRayQuerySupported: () -> boolean,
	createBottomLevel: (
		geometries: { RaytracingTriangleGeometry | RaytracingAabbGeometry },
		flags: number?
	) -> RaytracingAccelerationStructure,
	createTopLevel: (instances: { RaytracingInstance }, flags: number?) -> RaytracingAccelerationStructure,
	dispatch: (pipeline: RaytracingPipeline, info: RaytracingDispatchInfo) -> nil,
	setComputeAccelerationStructure: (slotIndex: number, accelerationStructure: RaytracingAccelerationStructure) -> nil,
	resetComputeAccelerationStructure: (slotIndex: number) -> nil,
	resetAllComputeAccelerationStructures: () -> nil,
	setGraphicsAccelerationStructure: (
		slotIndex: number,
		accelerationStructure: RaytracingAccelerationStructure
	) -> nil,
	resetGraphicsAccelerationStructure: (slotIndex: number) -> nil,
	resetAllGraphicsAccelerationStructures: () -> nil,
}

export type Collider = {
	isValid: (self: Collider) -> boolean,
	isAdded: (self: Collider) -> boolean,
	getPosition: (self: Collider) -> Vec3,
	getRotation: (self: Collider) -> Vec3,
	getVelocity: (self: Collider) -> Vec3,
	getAngularVelocity: (self: Collider) -> Vec3,
	setPosition: (self: Collider, position: Vec3, activate: boolean?) -> nil,
	setRotation: (self: Collider, rotation: Vec3, activate: boolean?) -> nil,
	setPositionAndRotation: (self: Collider, position: Vec3, rotation: Vec3, activate: boolean?) -> nil,
	setLinearVelocity: (self: Collider, velocity: Vec3) -> nil,
	setAngularVelocity: (self: Collider, velocity: Vec3) -> nil,
	addForce: (self: Collider, force: Vec3, activate: boolean?) -> nil,
	addImpulse: (self: Collider, impulse: Vec3) -> nil,
	setFriction: (self: Collider, friction: number) -> nil,
	setRestitution: (self: Collider, restitution: number) -> nil,
	activate: (self: Collider) -> nil,
	deactivate: (self: Collider) -> nil,
	remove: (self: Collider) -> nil,
	destroy: (self: Collider) -> nil,
}

export type World3D = {
	new: () -> World3D,
	newBoxCollider: (self: World3D, transform: Transform, isStatic: boolean) -> Collider,
	newSphereCollider: (self: World3D, position: Vec3, radius: number, isStatic: boolean) -> Collider,
	newCylinderCollider: (
		self: World3D,
		position: Vec3,
		rotation: Vec3,
		halfHeight: number,
		radius: number,
		isStatic: boolean
	) -> Collider,
	newCapsuleCollider: (
		self: World3D,
		position: Vec3,
		rotation: Vec3,
		halfHeight: number,
		radius: number,
		isStatic: boolean
	) -> Collider,

	setGravity: (self: World3D, gravity: Vec3) -> nil,
	getGravity: (self: World3D) -> Vec3,

	bodyCount: (self: World3D) -> number,
	addCollider: (self: World3D, collider: Collider, isActive: boolean) -> nil,
	removeCollider: (self: World3D, collider: Collider) -> nil,
	destroyCollider: (self: World3D, collider: Collider) -> nil,

	optimizeBroadPhase: (self: World3D) -> nil,
	update: (self: World3D, time: number, collisionSteps: number) -> nil,
}

export type Event = {
	quit: () -> nil,
}

export type Script = {
	load: (path: string, basePath: string?) -> nil,
}

export type Random = {
	getRange: (a: number, b: number) -> number,
	getIntRange: (a: number, b: number) -> number,
}

export type Window = {
	getName: () -> string,
	size: () -> Vec2,
	half: () -> Vec2,
	resize: (size: Vec2) -> nil,
	setFullscreen: (full: boolean) -> nil,
	rename: (name: string) -> nil,
	resized: () -> boolean,
	topLeft: () -> Vec2,
	topCenter: () -> Vec2,
	topRight: () -> Vec2,
	left: () -> Vec2,
	center: () -> Vec2,
	right: () -> Vec2,
	bottomLeft: () -> Vec2,
	bottomCenter: () -> Vec2,
	bottomRight: () -> Vec2,
}

export type BuiltinShader = {
	getDefaultVS: () -> Shader,
	getDefaultFS: () -> Shader,
	getDefaultInstancedVS: () -> Shader,
	getCubemapVS: () -> Shader,
	getCubemapFS: () -> Shader,
}

export type BuiltinPipelines = {
	getDefault3D: () -> GraphicsPipeline,
	getInstanced3D: () -> GraphicsPipeline,
	getDefault2D: () -> GraphicsPipeline,
	getCubemap: () -> GraphicsPipeline,
}

export type Graphics = {
	-- 2D drawing uses Processing/raylib-style screen coordinates:
	-- (0, 0) is the top-left of the window, x grows right, y grows down.
	-- drawRect/drawImage use Rect.x/y as the object's top-left corner.
	drawRect: (rect: Rect, color: Color, angle: number?) -> nil,
	drawImage: (texture: Texture, rect: Rect, angle: number?) -> nil,
	-- drawText positions the top-left of the rendered text.
	drawText: (text: string, font: Font, position: Vec2, color: Color?, fontSize: number?, angle: number?) -> nil,
	begin2D: (camera: Camera2D?) -> nil,
	begin3D: (camera: Camera3D) -> nil,
	finish: () -> nil,
	drawModel: (model: Model, transform: Transform) -> nil,
	drawModelInstanced: (model: Model, transforms: { Transform }) -> nil,
	drawCubemap: (cubemap: Texture) -> nil,
	getClearColor: () -> Color,
	setClearColor: (c: Color) -> nil,
	setGraphicsPipeline: (pipe: GraphicsPipeline) -> nil,
	resetGraphicsPipeline: () -> nil,
	setTexture: (slotIndex: number, texture: Texture) -> nil,
	resetTexture: (slotIndex: number) -> nil,
	resetAllTexture: () -> nil,
	setUniformBuffer: (binding: number, data: Buffer) -> nil,
	beginRenderTarget: (rt: RenderTexture) -> nil,
	endRenderTarget: () -> nil,
	readbackTexture: (rt: RenderTexture, out: Texture) -> Texture,
}

export type Compute = {
	setComputePipeline: (pipe: ComputePipeline) -> nil,
	resetComputePipeline: () -> nil,
	setUniformBuffer: (slotIndex: number, data: Buffer) -> nil,
	setStorageBuffer: (slotIndex: number, buffer: ComputeBuffer) -> nil,
	resetStorageBuffer: (slotIndex: number) -> nil,
	resetAllStorageBuffers: () -> nil,
	dispatch: (groupCountX: number, groupCountY: number?, groupCountZ: number?) -> nil,
}

export type Gui = {
	setFont: (font: Font) -> nil,
	setFontSize: (size: number) -> nil,
	setThemeColor: (background: Color, hover: Color, active: Color, text: Color, accent: Color) -> nil,
	label: (text: string, position: Vec2, color: Color?, fontSize: number?) -> nil,
	button: (text: string, rect: Rect) -> boolean,
	checkbox: (text: string, checked: boolean, rect: Rect) -> boolean,
	sliderFloat: (text: string, value: number, min: number, max: number, rect: Rect) -> number,
	scrollVertical: (scroll: number, viewport: Rect, contentHeight: number, wheelStep: number?) -> number,
}

export type Physics = {
	setGravity: (gravity: Vec3) -> nil,
	getGravity: () -> Vec3,
	bodyCount: () -> number,
	optimizeBroadPhase: () -> nil,
}

export type Scancode = {
	A: Scancode,
	B: Scancode,
	C: Scancode,
	D: Scancode,
	E: Scancode,
	F: Scancode,
	G: Scancode,
	H: Scancode,
	I: Scancode,
	J: Scancode,
	K: Scancode,
	L: Scancode,
	M: Scancode,
	N: Scancode,
	O: Scancode,
	P: Scancode,
	Q: Scancode,
	R: Scancode,
	S: Scancode,
	T: Scancode,
	U: Scancode,
	V: Scancode,
	W: Scancode,
	X: Scancode,
	Y: Scancode,
	Z: Scancode,
	Key0: Scancode,
	Key1: Scancode,
	Key2: Scancode,
	Key3: Scancode,
	Key4: Scancode,
	Key5: Scancode,
	Key6: Scancode,
	Key7: Scancode,
	Key8: Scancode,
	Key9: Scancode,
	F1: Scancode,
	F2: Scancode,
	F3: Scancode,
	F4: Scancode,
	F5: Scancode,
	F6: Scancode,
	F7: Scancode,
	F8: Scancode,
	F9: Scancode,
	F10: Scancode,
	F11: Scancode,
	F12: Scancode,
	UP: Scancode,
	DOWN: Scancode,
	LEFT: Scancode,
	RIGHT: Scancode,
	ESCAPE: Scancode,
	SPACE: Scancode,
	BACKSPACE: Scancode,
	TAB: Scancode,
	RETURN: Scancode,
	LSHIFT: Scancode,
	RSHIFT: Scancode,
	LCTRL: Scancode,
	RCTRL: Scancode,
	LALT: Scancode,
	ENTER: Scancode,
}

export type Keyboard = {
	isPressed: (scancode: Scancode) -> boolean,
	isReleased: (scancode: Scancode) -> boolean,
	isDown: (scancode: Scancode) -> boolean,
}

export type Mouse = {
	LEFT: number,
	RIGHT: number,
	MIDDLE: number,
	X1: number,
	X2: number,
	isPressed: (btn: number) -> boolean,
	isReleased: (btn: number) -> boolean,
	isDown: (btn: number) -> boolean,
	getPosition: () -> Vec2,
	getPositionOnScene: () -> Vec2,
	setPosition: (pos: Vec2) -> nil,
	setPositionOnScene: (pos: Vec2) -> nil,
	getScrollWheel: () -> Vec2,
	hideCursor: (isHide: boolean) -> nil,
	setRelative: (isRelative: boolean) -> nil,
}

export type GamepadButton = {
	INVALID: GamepadButton,
	A: GamepadButton,
	B: GamepadButton,
	X: GamepadButton,
	Y: GamepadButton,
	BACK: GamepadButton,
	GUIDE: GamepadButton,
	START: GamepadButton,
	LEFTSTICK: GamepadButton,
	RIGHTSTICK: GamepadButton,
	LEFTSHOULDER: GamepadButton,
	RIGHTSHOULDER: GamepadButton,
	DPAD_UP: GamepadButton,
	DPAD_DOWN: GamepadButton,
	DPAD_LEFT: GamepadButton,
	DPAD_RIGHT: GamepadButton,
	MISC1: GamepadButton,
	PADDLE1: GamepadButton,
	PADDLE2: GamepadButton,
	PADDLE3: GamepadButton,
	PADDLE4: GamepadButton,
	TOUCHPAD: GamepadButton,
}

export type Gamepad = {
	isPressed: (btn: GamepadButton) -> boolean,
	isReleased: (btn: GamepadButton) -> boolean,
	isDown: (btn: GamepadButton) -> boolean,
	getLeftStick: () -> Vec2,
	getRightStick: () -> Vec2,
	isConnected: () -> boolean,
}

export type Periodic = {
	sin0_1: (t1: number, t2: number) -> number,
	cos0_1: (t1: number, t2: number) -> number,
}

export type Time = {
	seconds: () -> number,
	milli: () -> number,
	delta: () -> number,
}

export type Filesystem = {
	enumerateDirectory: (path: string) -> { string },
	read: (path: string) -> Buffer?,
	exists: (path: string) -> boolean,
	write: (path: string, data: Buffer) -> boolean,
	getUserDirectory: () -> string,
}

export type VideoInfo = {
	width: number,
	height: number,
	fps: number,
	frameCount: number,
}

export type VideoFrame = {
	width: number,
	height: number,
	channels: number,
	pixels: Buffer,
}

export type VideoWriter = {
	new: () -> VideoWriter,
	open: (self: VideoWriter, path: string, width: number, height: number, fps: number) -> boolean,
	addFrame: (
		self: VideoWriter,
		pixels: Buffer,
		width: number,
		height: number,
		format: string?,
		quality: number?,
		strideBytes: number?
	) -> boolean,
	addEncodedFrame: (self: VideoWriter, encoded: Buffer) -> boolean,
	close: (self: VideoWriter) -> boolean,
	frameCount: (self: VideoWriter) -> number,
	lastError: (self: VideoWriter) -> string,
}

export type VideoReader = {
	new: () -> VideoReader,
	open: (self: VideoReader, path: string) -> boolean,
	close: (self: VideoReader) -> nil,
	info: (self: VideoReader) -> VideoInfo,
	readEncodedFrame: (self: VideoReader, index: number) -> Buffer?,
	readFrame: (self: VideoReader, index: number) -> VideoFrame?,
	lastError: (self: VideoReader) -> string,
}

export type Log = {
	verbose: (msg: any) -> nil,
	info: (msg: any) -> nil,
	error: (msg: any) -> nil,
	warn: (msg: any) -> nil,
	critical: (msg: any) -> nil,
}

export type Synth = {
	new: () -> Synth,
	setBpm: (self: Synth, bpm: number) -> nil,
	setStepBeats: (self: Synth, stepBeats: number) -> nil,
	setADSR: (self: Synth, a: number, d: number, s: number, r: number) -> nil,
	setMasterGain: (self: Synth, gain: number) -> nil,
	setPattern: (self: Synth, pattern: string) -> nil,
	play: (self: Synth) -> nil,
	stop: (self: Synth) -> nil,
}

return {
	Buffer = {} :: Buffer,
	Vec3 = Vec3Impl,
	Vec2 = Vec2Impl,
	Texture = {} :: Texture,
	RenderTexture = {} :: RenderTexture,
	Sound = {} :: Sound,
	Camera3D = {} :: Camera3D,
	Camera2D = {} :: Camera2D,
	TextureKey = {} :: TextureKey,
	Model = {} :: Model,
	MeshBuilder = {} :: MeshBuilder,
	AABB = {} :: AABB,
	Timer = {} :: Timer,
	Shader = {} :: Shader,
	ShaderBundle = {} :: ShaderBundle,
	Font = {} :: Font,
	Color = {} :: Color,
	GraphicsPipeline = {} :: GraphicsPipeline,
	ComputeBuffer = {} :: ComputeBuffer,
	ComputePipeline = {} :: ComputePipeline,
	Raytracing = {} :: Raytracing,
	Pivot = {} :: Pivot,
	Rect = {} :: Rect,
	Transform = {} :: Transform,
	Collider = {} :: Collider,
	World3D = {} :: World3D,
	Synth = {} :: Synth,
	Event = {} :: Event,
	Script = {} :: Script,
	Filesystem = {} :: Filesystem,
	Random = {} :: Random,
	Window = {} :: Window,
	ShaderStage = {} :: ShaderStage,
	ShaderFormat = {} :: ShaderFormat,
	BuiltinShader = {} :: BuiltinShader,
	BuiltinPipelines = {} :: BuiltinPipelines,
	Graphics = {} :: Graphics,
	Compute = {} :: Compute,
	Gui = {} :: Gui,
	Physics = {} :: Physics,
	Scancode = {} :: Scancode,
	Keyboard = {} :: Keyboard,
	Mouse = {} :: Mouse,
	GamepadButton = {} :: GamepadButton,
	Gamepad = {} :: Gamepad,
	Periodic = {} :: Periodic,
	Time = {} :: Time,
	Log = {} :: Log,
	VideoWriter = {} :: VideoWriter,
	VideoReader = {} :: VideoReader,
}
