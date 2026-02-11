--!strict

-- This file is a Luau definition file for Sinen's Lua API.

export type Buffer = {
	new: (array: any) -> Buffer,
}

export type Vec3 = {
	x: number,
	y: number,
	z: number,

	new: (((x: number, y: number, z: number) -> Vec3) & ((value: number) -> Vec3)),
	copy: (self: Vec3) -> Vec3,
	length: (self: Vec3) -> number,
	forward: (self: Vec3, rotation: Vec3) -> Vec3,
	normalize: (self: Vec3) -> Vec3,
	dot: (self: Vec3, other: Vec3) -> number,
	cross: (self: Vec3, other: Vec3) -> Vec3,
	lerp: (self: Vec3, other: Vec3, t: number) -> Vec3,
	reflect: (self: Vec3, n: Vec3) -> Vec3,
}

export type Vec2 = {
	x: number,
	y: number,

	new: (((x: number, y: number) -> Vec2) & ((value: number) -> Vec2)),
	copy: (self: Vec2) -> Vec2,
	length: (self: Vec2) -> number,
	normalize: (self: Vec2) -> Vec2,
	dot: (self: Vec2, other: Vec2) -> number,
	lerp: (self: Vec2, other: Vec2, t: number) -> Vec2,
	reflect: (self: Vec2, n: Vec2) -> Vec2,
}

export type Pivot = {
	TopLeft: number,
	TopCenter: number,
	TopRight: number,
	Center: number,
	BottomLeft: number,
	BottomCenter: number,
	BottomRight: number,
}

export type Rect = {
	x: number,
	y: number,
	width: number,
	height: number,

	new: (((x: number, y: number, width: number, height: number) -> Rect) & ((position: Vec2, size: Vec2) -> Rect) & ((pivot: Pivot, x: number, y: number, w: number, h: number) -> Rect) & ((pivot: Pivot, pos: Vec2, size: Vec2) -> Rect)),
	topLeft: (self: Rect) -> Vec2,
	topCenter: (self: Rect) -> Vec2,
	topRight: (self: Rect) -> Vec2,
	bottomLeft: (self: Rect) -> Vec2,
	bottomCenter: (self: Rect) -> Vec2,
	bottomRight: (self: Rect) -> Vec2,
	position: (self: Rect) -> Vec2,
	positionFromPivot: (self: Rect, pivot: Pivot) -> Vec2,
	size: (self: Rect) -> Vec2,
	intersectsRect: (self: Rect, rect: Rect) -> boolean,
}

export type Transform = {
	position: Vec3,
	rotation: Vec3,
	scale: Vec3,
	new: () -> Transform,
}

export type AABB = {
	new: () -> AABB,
	min: Vec3,
	max: Vec3,
	updateWorld: (self: AABB, position: Vec3, scale: Vec3, modelAABB: AABB) -> nil,
	intersectsAABB: (self: AABB, aabb: AABB) -> boolean,
}

export type Color = {
	r: number,
	g: number,
	b: number,
	a: number,

	new: (((r: number, g: number, b: number, a: number) -> Color) & ((value: number, a: number) -> Color) & ((value: number) -> Color) & (() -> Color)),
}

export type Texture = {
	new: ((() -> Texture) & ((width: number, height: number) -> Texture)),
	fill: (self: Texture, color: Color) -> nil,
	blend: (self: Texture, color: Color) -> nil,
	copy: (self: Texture) -> Texture,
	load: (self: Texture, path: string) -> nil,
	loadCubemap: (self: Texture, path: string) -> nil,
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
	load: (((self: Model, path: string) -> nil) & ((self: Model, buffer: Buffer) -> nil)),
	loadSprite: (self: Model) -> nil,
	loadBox: (self: Model) -> nil,
	getBoneUniformBuffer: (self: Model) -> Buffer,
	play: (self: Model, position: number) -> nil,
	update: (self: Model, delta: number) -> nil,
	hasTexture: (self: Model, textureKey: number) -> boolean,
	getTexture: (self: Model, textureKey: number) -> Texture,
	setTexture: (self: Model, textureKey: number, texture: Texture) -> nil,
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
	Vertex: number,
	Fragment: number,
	Compute: number,
}

export type Shader = {
	new: () -> Shader,
	load: (self: Shader, path: string, shaderStage: ShaderStage, uniform_count: number) -> nil,
	compileAndLoad: (self: Shader, source: string, shaderStage: ShaderStage) -> nil,
}

export type Font = {
	new: () -> Font,
	load: (((self: Font, size: number) -> nil) & ((self: Font, size: number, path: string) -> nil) & ((self: Font, size: number, buffer: Buffer) -> nil)),
	resize: (self: Font, size: number) -> nil,
	region: (self: Font, text: string, fontSize: number, pivot: number, position: Vec2) -> Rect,
}

export type Grid = {
	new: (w: number, h: number) -> Grid,
	at: (self: Grid, x: number, y: number) -> number,
	set: (self: Grid, x: number, y: number, v: number) -> nil,
	width: (self: Grid) -> number,
	height: (self: Grid) -> number,
	size: (self: Grid) -> number,
	clear: (self: Grid) -> nil,
	resize: (self: Grid, w: number, h: number) -> nil,
	fill: (self: Grid, value: number) -> nil,
	fillRect: (self: Grid, index: number, value: number) -> nil,
	setRow: (self: Grid, index: number, value: number) -> nil,
	setColumn: (self: Grid, index: number, value: number) -> nil,
}

export type BFSGrid = {
	new: (grid: Grid) -> BFSGrid,
	width: (self: BFSGrid) -> number,
	height: (self: BFSGrid) -> number,
	findPath: (self: BFSGrid, start: Vec2, end_: Vec2) -> any,
	trace: (self: BFSGrid) -> Vec2,
	traceable: (self: BFSGrid) -> boolean,
	reset: (self: BFSGrid) -> nil,
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

export type World = {
	new: () -> World,
	createBoxCollider: (self: World, transform: Transform, isStatic: boolean) -> Collider,
	createSphereCollider: (self: World, position: Vec3, radius: number, isStatic: boolean) -> Collider,
	createCylinderCollider: (self: World, position: Vec3, rotation: Vec3, halfHeight: number, radius: number, isStatic: boolean) -> Collider,
	createCapsuleCollider: (self: World, position: Vec3, rotation: Vec3, halfHeight: number, radius: number, isStatic: boolean) -> Collider,
	addCollider: (self: World, collider: Collider, isActive: boolean) -> nil,
	removeCollider: (self: World, collider: Collider) -> nil,
	destroyCollider: (self: World, collider: Collider) -> nil,
}

export type Arguments = {
	getArgc: () -> number,
	getArgv: () -> { string },
}

export type Event = {
	quit: () -> nil,
}

export type Script = {
	load: (path: string, basePath: string?) -> nil,
}

export type Filesystem = {
	enumerateDirectory: (path: string) -> any,
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
	drawRect: (rect: Rect, color: Color, angle: number?) -> nil,
	drawImage: (texture: Texture, rect: Rect, angle: number?) -> nil,
	drawText: (text: string, font: Font, position: Vec2, color: Color?, fontSize: number?, angle: number?) -> nil,
	drawModel: (model: Model, transform: Transform) -> nil,
	drawModelInstanced: (model: Model, transforms: { Transform }) -> nil,
	drawCubemap: (cubemap: Texture) -> nil,
	setCamera3D: (camera: Camera3D) -> nil,
	getCamera3D: () -> Camera3D,
	setCamera2D: (camera: Camera2D) -> nil,
	getCamera2D: () -> Camera2D,
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

export type Physics = {
	setGravity: (gravity: Vec3) -> nil,
	getGravity: () -> Vec3,
	bodyCount: () -> number,
	optimizeBroadPhase: () -> nil,
}

export type Keyboard = {
	A: number,
	B: number,
	C: number,
	D: number,
	E: number,
	F: number,
	G: number,
	H: number,
	I: number,
	J: number,
	K: number,
	L: number,
	M: number,
	N: number,
	O: number,
	P: number,
	Q: number,
	R: number,
	S: number,
	T: number,
	U: number,
	V: number,
	W: number,
	X: number,
	Y: number,
	Z: number,
	Key0: number,
	Key1: number,
	Key2: number,
	Key3: number,
	Key4: number,
	Key5: number,
	Key6: number,
	Key7: number,
	Key8: number,
	Key9: number,
	F1: number,
	F2: number,
	F3: number,
	F4: number,
	F5: number,
	F6: number,
	F7: number,
	F8: number,
	F9: number,
	F10: number,
	F11: number,
	F12: number,
	UP: number,
	DOWN: number,
	LEFT: number,
	RIGHT: number,
	ESCAPE: number,
	SPACE: number,
	BACKSPACE: number,
	TAB: number,
	RETURN: number,
	LSHIFT: number,
	RSHIFT: number,
	LCTRL: number,
	RCTRL: number,
	LALT: number,

	isPressed: (scancode: number) -> boolean,
	isReleased: (scancode: number) -> boolean,
	isDown: (scancode: number) -> boolean,
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
	getScrollWheel: () -> number,
	hideCursor: (isHide: boolean) -> nil,
	setRelative: (isRelative: boolean) -> nil,
}

export type Gamepad = {
	INVALID: number,
	A: number,
	B: number,
	X: number,
	Y: number,
	BACK: number,
	GUIDE: number,
	START: number,
	LEFTSTICK: number,
	RIGHTSTICK: number,
	LEFTSHOULDER: number,
	RIGHTSHOULDER: number,
	DPAD_UP: number,
	DPAD_DOWN: number,
	DPAD_LEFT: number,
	DPAD_RIGHT: number,
	MISC1: number,
	PADDLE1: number,
	PADDLE2: number,
	PADDLE3: number,
	PADDLE4: number,
	TOUCHPAD: number,
	isPressed: (btn: number) -> boolean,
	isReleased: (btn: number) -> boolean,
	isDown: (btn: number) -> boolean,
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

export type Log = {
	verbose: (msg: any) -> nil,
	info: (msg: any) -> nil,
	error: (msg: any) -> nil,
	warn: (msg: any) -> nil,
	critical: (msg: any) -> nil,
}

export type ImGuiWindowFlags = {
	None: number,
	NoTitleBar: number,
	NoResize: number,
	NoMove: number,
	NoScrollBar: number,
	NoScrollWithMouse: number,
	NoCollapse: number,
	AlwaysAutoResize: number,
	NoBackground: number,
	NoSavedSettings: number,
	NoMouseInputs: number,
	MenuBar: number,
	HorizontalScrollbar: number,
	NoFocusOnAppearing: number,
	NoBringToFrontOnFocus: number,
	AlwaysVerticalScrollbar: number,
	AlwaysHorizontalScrollbar: number,
	NoNavInputs: number,
	NoNavFocus: number,
	UnsavedDocument: number,
	NoNav: number,
	NoDecoration: number,
	NoInputs: number,
}

export type ImGui = {
	Begin: (name: string, flags: ImGuiWindowFlags?) -> nil,
	End: () -> nil,
	button: (name: string) -> nil,
	text: (name: string) -> nil,
	setNextWindowPos: (pos: Vec2) -> nil,
	setNextWindowSize: (size: Vec2) -> nil,
	WindowFlags: ImGuiWindowFlags,
}

export type sn = {
	Buffer: Buffer,
	Vec3: Vec3,
	Vec2: Vec2,
	Texture: Texture,
	RenderTexture: RenderTexture,
	Sound: Sound,
	Camera3D: Camera3D,
	Camera2D: Camera2D,
	TextureKey: TextureKey,
	Model: Model,
	AABB: AABB,
	Timer: Timer,
	Shader: Shader,
	Font: Font,
	Color: Color,
	Grid: Grid,
	BFSGrid: BFSGrid,
	GraphicsPipeline: GraphicsPipeline,
	Pivot: Pivot,
	Rect: Rect,
	Transform: Transform,
	Collider: Collider,
	World: World,
	Arguments: Arguments,
	Event: Event,
	Script: Script,
	Filesystem: Filesystem,
	Random: Random,
	Window: Window,
	ShaderStage: ShaderStage,
	BuiltinShader: BuiltinShader,
	BuiltinPipelines: BuiltinPipelines,
	Graphics: Graphics,
	Physics: Physics,
	Keyboard: Keyboard,
	Mouse: Mouse,
	Gamepad: Gamepad,
	Periodic: Periodic,
	Time: Time,
	Log: Log,
	ImGui: ImGui,
}

declare sn: sn
