# Chemins du projet
SRC_DIR   = ..\src
BUILD_DIR = ..\build

# Chemins Borland C++ 3.1
BC_PATH  = C:\BORLANDC
INCLUDES = -I$(BC_PATH)\INCLUDE -I$(SRC_DIR)
LIBS     = -L$(BC_PATH)\LIB
BGI      = $(BC_PATH)\BGI

# Indique à Borland Make où chercher les fichiers sources .cpp
.path.cpp = $(SRC_DIR)

# Par défaut, si DEBUG n'est pas défini, on est en Release
!ifdef DEBUG
CFLAGS = -ml -v -y -O- -N $(INCLUDES)    # Model Large, Debug info, Pas d'optimisation
!else
CFLAGS = -ml -v- -O2 -vi -2 -y- $(INCLUDES) # Model Large, Pas de debug info, Optimisation vitesse
!endif

TARGET = $(BUILD_DIR)\doscube.exe

# Fichiers objets générés dans le dossier build
BUILD_OBJS = $(BUILD_DIR)\main.obj \
			 $(BUILD_DIR)\cube.obj \
 			 $(BUILD_DIR)\renderer.obj \
			 $(BUILD_DIR)\mouse.obj

# 1. RÈGLE DE LIAISON FINALE
$(TARGET): $(BUILD_OBJS)
	bcc $(CFLAGS) $(LIBS) -e$(TARGET) @&&|
$(BUILD_OBJS) graphics.lib
|


# 2. RÈGLE GÉNÉRIQUE DE COMPILATION (.cpp -> .obj dans BUILD_DIR)
.cpp.obj:
	bcc $(CFLAGS) -n$(BUILD_DIR) -c $<

# Nettoyage complet
clean:
	del $(BUILD_DIR)\*.obj
	del $(TARGET)
	del $(BUILD_DIR)\log.txt