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
.autodepend

# Par défaut, si DEBUG n'est pas défini, on est en Release
!ifdef DEBUG
CFLAGS = -mm -v -y -O- -N $(INCLUDES)   # Debug info, pas d'optimisation, test de pile
!else
CFLAGS = -mm -v- -O2 -G -vi -2 -d -y- $(INCLUDES)   # Medium, vitesse, code 286
!endif

TARGET = $(BUILD_DIR)\doscube.exe

# Fichiers objets générés dans le dossier build
BUILD_OBJS = $(BUILD_DIR)\main.obj \
			 $(BUILD_DIR)\cube.obj \
 			 $(BUILD_DIR)\renderer.obj \
			 $(BUILD_DIR)\mouse.obj \
			 $(BUILD_DIR)\control.obj \
			 $(BUILD_DIR)\lang.obj \
			 $(BUILD_DIR)\timer.obj \
			 $(BUILD_DIR)\history.obj \
			 $(BUILD_DIR)\options.obj \
			 $(BUILD_DIR)\cpu.obj \
			 $(BUILD_DIR)\about.obj

# 1. RÈGLE DE LIAISON FINALE
$(TARGET): $(BUILD_OBJS)
	bcc $(CFLAGS) $(LIBS) -e$(TARGET) @&&|
$(BUILD_OBJS)
|


# 2. RÈGLE GÉNÉRIQUE DE COMPILATION (.cpp -> .obj dans BUILD_DIR)
.cpp.obj:
	bcc $(CFLAGS) -n$(BUILD_DIR) -c $<

# Nettoyage complet
clean:
	del $(BUILD_DIR)\*.obj
	del $(TARGET)
	del $(BUILD_DIR)\log.txt