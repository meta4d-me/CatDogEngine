import os
texture  = "textures/"
compiled = "compiled/"

baseColor = "aseColor"
normal    = "ormal"
roughness = "oughness"

for file in os.listdir(texture):
	print("compiling : "+file)
	textureName = os.path.splitext(file)[0]
	textureType = os.path.splitext(file)[1]
	if baseColor in file:
		os.system("texturec -f "+texture+textureName+textureType+" -o "+compiled+textureName+".dds -t BC3 -m -q h")
	elif normal in file:
		os.system("texturec -f "+texture+textureName+textureType+" -o "+compiled+textureName+".dds -t BC3 -m -q h -n")
	elif roughness in file:
		os.system("texturec -f "+texture+textureName+textureType+" -o "+compiled+textureName+".dds -t BC3 -m -q h --linear")
	else:
		print("Undeclared texture type!\n")
