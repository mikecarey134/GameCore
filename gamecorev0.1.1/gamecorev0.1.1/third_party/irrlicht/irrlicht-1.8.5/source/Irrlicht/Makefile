VERSION_MAJOR = 1
VERSION_MINOR = 8
VERSION_RELEASE = 5

# Irrlicht Engine 1.8.5
# Makefile for Linux, Windows cross-compilation, and macOS Command Line Tools.

# ---------------------------------------------------------------------------
# Source object lists
# ---------------------------------------------------------------------------

IRRMESHLOADER = CBSPMeshFileLoader.o CMD2MeshFileLoader.o CMD3MeshFileLoader.o CMS3DMeshFileLoader.o CB3DMeshFileLoader.o C3DSMeshFileLoader.o COgreMeshFileLoader.o COBJMeshFileLoader.o CColladaFileLoader.o CCSMLoader.o CDMFLoader.o CLMTSMeshFileLoader.o CMY3DMeshFileLoader.o COCTLoader.o CXMeshFileLoader.o CIrrMeshFileLoader.o CSTLMeshFileLoader.o CLWOMeshFileLoader.o CPLYMeshFileLoader.o CSMFMeshFileLoader.o

IRRMESHWRITER = CColladaMeshWriter.o CIrrMeshWriter.o CSTLMeshWriter.o COBJMeshWriter.o CPLYMeshWriter.o

IRRMESHOBJ = $(IRRMESHLOADER) $(IRRMESHWRITER) \
	CSkinnedMesh.o CBoneSceneNode.o CMeshSceneNode.o \
	CAnimatedMeshSceneNode.o CAnimatedMeshMD2.o CAnimatedMeshMD3.o \
	CQ3LevelMesh.o CQuake3ShaderSceneNode.o CAnimatedMeshHalfLife.o

IRROBJ = CBillboardSceneNode.o CCameraSceneNode.o CDummyTransformationSceneNode.o CEmptySceneNode.o CGeometryCreator.o CLightSceneNode.o CMeshManipulator.o CMetaTriangleSelector.o COctreeSceneNode.o COctreeTriangleSelector.o CSceneCollisionManager.o CSceneManager.o CShadowVolumeSceneNode.o CSkyBoxSceneNode.o CSkyDomeSceneNode.o CTerrainSceneNode.o CTerrainTriangleSelector.o CVolumeLightSceneNode.o CCubeSceneNode.o CSphereSceneNode.o CTextSceneNode.o CTriangleBBSelector.o CTriangleSelector.o CWaterSurfaceSceneNode.o CMeshCache.o CDefaultSceneNodeAnimatorFactory.o CDefaultSceneNodeFactory.o CSceneLoaderIrr.o

IRRPARTICLEOBJ = CParticleAnimatedMeshSceneNodeEmitter.o CParticleBoxEmitter.o CParticleCylinderEmitter.o CParticleMeshEmitter.o CParticlePointEmitter.o CParticleRingEmitter.o CParticleSphereEmitter.o CParticleAttractionAffector.o CParticleFadeOutAffector.o CParticleGravityAffector.o CParticleRotationAffector.o CParticleSystemSceneNode.o CParticleScaleAffector.o

IRRANIMOBJ = CSceneNodeAnimatorCameraFPS.o CSceneNodeAnimatorCameraMaya.o CSceneNodeAnimatorCollisionResponse.o CSceneNodeAnimatorDelete.o CSceneNodeAnimatorFlyCircle.o CSceneNodeAnimatorFlyStraight.o CSceneNodeAnimatorFollowSpline.o CSceneNodeAnimatorRotation.o CSceneNodeAnimatorTexture.o

IRRDRVROBJ = CNullDriver.o COpenGLDriver.o COpenGLNormalMapRenderer.o COpenGLParallaxMapRenderer.o COpenGLShaderMaterialRenderer.o COpenGLTexture.o COpenGLSLMaterialRenderer.o COpenGLExtensionHandler.o CD3D8Driver.o CD3D8NormalMapRenderer.o CD3D8ParallaxMapRenderer.o CD3D8ShaderMaterialRenderer.o CD3D8Texture.o CD3D9Driver.o CD3D9HLSLMaterialRenderer.o CD3D9NormalMapRenderer.o CD3D9ParallaxMapRenderer.o CD3D9ShaderMaterialRenderer.o CD3D9Texture.o

IRRIMAGEOBJ = CColorConverter.o CImage.o CImageLoaderBMP.o CImageLoaderDDS.o CImageLoaderJPG.o CImageLoaderPCX.o CImageLoaderPNG.o CImageLoaderPSD.o CImageLoaderTGA.o CImageLoaderPPM.o CImageLoaderWAL.o CImageLoaderRGB.o \
	CImageWriterBMP.o CImageWriterJPG.o CImageWriterPCX.o CImageWriterPNG.o CImageWriterPPM.o CImageWriterPSD.o CImageWriterTGA.o

IRRVIDEOOBJ = CVideoModeList.o CFPSCounter.o $(IRRDRVROBJ) $(IRRIMAGEOBJ)

IRRSWRENDEROBJ = CSoftwareDriver.o CSoftwareTexture.o CTRFlat.o CTRFlatWire.o CTRGouraud.o CTRGouraudWire.o CTRNormalMap.o CTRStencilShadow.o CTRTextureFlat.o CTRTextureFlatWire.o CTRTextureGouraud.o CTRTextureGouraudAdd.o CTRTextureGouraudNoZ.o CTRTextureGouraudWire.o CZBuffer.o CTRTextureGouraudVertexAlpha2.o CTRTextureGouraudNoZ2.o CTRTextureLightMap2_M2.o CTRTextureLightMap2_M4.o CTRTextureLightMap2_M1.o CSoftwareDriver2.o CSoftwareTexture2.o CTRTextureGouraud2.o CTRGouraud2.o CTRGouraudAlpha2.o CTRGouraudAlphaNoZ2.o CTRTextureDetailMap2.o CTRTextureGouraudAdd2.o CTRTextureGouraudAddNoZ2.o CTRTextureWire2.o CTRTextureLightMap2_Add.o CTRTextureLightMapGouraud2_M4.o IBurningShader.o CTRTextureBlend.o CTRTextureGouraudAlpha.o CTRTextureGouraudAlphaNoZ.o CDepthBuffer.o CBurningShader_Raster_Reference.o

IRRIOOBJ = CFileList.o CFileSystem.o CLimitReadFile.o CMemoryFile.o CReadFile.o CWriteFile.o CXMLReader.o CXMLWriter.o CWADReader.o CZipReader.o CPakReader.o CNPKReader.o CTarReader.o CMountPointReader.o irrXML.o CAttributes.o lzma/LzmaDec.o

IRROTHEROBJ = CIrrDeviceSDL.o CIrrDeviceLinux.o CIrrDeviceConsole.o CIrrDeviceStub.o CIrrDeviceWin32.o CIrrDeviceFB.o CLogger.o COSOperator.o CIrrDeviceConsole.o Irrlicht.o os.o

IRRGUIOBJ = CGUIButton.o CGUICheckBox.o CGUIComboBox.o CGUIContextMenu.o CGUIEditBox.o CGUIEnvironment.o CGUIFileOpenDialog.o CGUIFont.o CGUIImage.o CGUIInOutFader.o CGUIListBox.o CGUIMenu.o CGUIMeshViewer.o CGUIMessageBox.o CGUIModalScreen.o CGUIScrollBar.o CGUISpinBox.o CGUISkin.o CGUIStaticText.o CGUITabControl.o CGUITable.o CGUIToolBar.o CGUIWindow.o CGUIColorSelectDialog.o CDefaultGUIElementFactory.o CGUISpriteBank.o CGUIImageList.o CGUITreeView.o

ZLIBOBJ = zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/deflate.o zlib/inffast.o zlib/inflate.o zlib/inftrees.o zlib/trees.o zlib/uncompr.o zlib/zutil.o

JPEGLIBOBJ = jpeglib/jcapimin.o jpeglib/jcapistd.o jpeglib/jccoefct.o jpeglib/jccolor.o jpeglib/jcdctmgr.o jpeglib/jchuff.o jpeglib/jcinit.o jpeglib/jcmainct.o jpeglib/jcmarker.o jpeglib/jcmaster.o jpeglib/jcomapi.o jpeglib/jcparam.o jpeglib/jcprepct.o jpeglib/jcsample.o jpeglib/jctrans.o jpeglib/jdapimin.o jpeglib/jdapistd.o jpeglib/jdatadst.o jpeglib/jdatasrc.o jpeglib/jdcoefct.o jpeglib/jdcolor.o jpeglib/jddctmgr.o jpeglib/jdhuff.o jpeglib/jdinput.o jpeglib/jdmarker.o jpeglib/jdmaster.o jpeglib/jdmainct.o jpeglib/jdmerge.o jpeglib/jdpostct.o jpeglib/jdsample.o jpeglib/jdtrans.o jpeglib/jerror.o jpeglib/jfdctflt.o jpeglib/jfdctfst.o jpeglib/jfdctint.o jpeglib/jidctflt.o jpeglib/jidctfst.o jpeglib/jidctint.o jpeglib/jmemmgr.o jpeglib/jmemnobs.o jpeglib/jquant1.o jpeglib/jquant2.o jpeglib/jutils.o jpeglib/jcarith.o jpeglib/jdarith.o jpeglib/jaricom.o

LIBPNGOBJ = libpng/png.o libpng/pngerror.o libpng/pngget.o libpng/pngmem.o libpng/pngpread.o libpng/pngread.o libpng/pngrio.o libpng/pngrtran.o libpng/pngrutil.o libpng/pngset.o libpng/pngtrans.o libpng/pngwio.o libpng/pngwrite.o libpng/pngwtran.o libpng/pngwutil.o

LIBAESGM = aesGladman/aescrypt.o aesGladman/aeskey.o aesGladman/aestab.o aesGladman/fileenc.o aesGladman/hmac.o aesGladman/prng.o aesGladman/pwd2key.o aesGladman/sha1.o aesGladman/sha2.o

BZIP2OBJ = bzip2/blocksort.o bzip2/huffman.o bzip2/crctable.o bzip2/randtable.o bzip2/bzcompress.o bzip2/decompress.o bzip2/bzlib.o

EXTRAOBJ =

# Linux/Windows source list.
LINKOBJ = $(IRRMESHOBJ) $(IRROBJ) $(IRRPARTICLEOBJ) $(IRRANIMOBJ) \
	$(IRRVIDEOOBJ) $(IRRSWRENDEROBJ) $(IRRIOOBJ) $(IRROTHEROBJ) \
	$(IRRGUIOBJ) $(ZLIBOBJ) $(JPEGLIBOBJ) $(LIBPNGOBJ) $(LIBAESGM) \
	$(BZIP2OBJ) $(EXTRAOBJ)

# macOS source list: OpenGL and macOS device only; no Direct3D/Linux/Windows.
IRRDRVROBJ_OSX = CNullDriver.o COpenGLDriver.o COpenGLNormalMapRenderer.o \
	COpenGLParallaxMapRenderer.o COpenGLShaderMaterialRenderer.o \
	COpenGLTexture.o COpenGLSLMaterialRenderer.o COpenGLExtensionHandler.o

IRRVIDEOOBJ_OSX = CVideoModeList.o CFPSCounter.o $(IRRDRVROBJ_OSX) $(IRRIMAGEOBJ)

IRROTHEROBJ_OSX = CIrrDeviceConsole.o CIrrDeviceStub.o CLogger.o COSOperator.o Irrlicht.o os.o \
	MacOSX/CIrrDeviceMacOSX.o MacOSX/OSXClipboard.o MacOSX/AppDelegate.o

LINKOBJ_OSX = $(IRRMESHOBJ) $(IRROBJ) $(IRRPARTICLEOBJ) $(IRRANIMOBJ) \
	$(IRRVIDEOOBJ_OSX) $(IRRSWRENDEROBJ) $(IRRIOOBJ) $(IRROTHEROBJ_OSX) \
	$(IRRGUIOBJ) $(ZLIBOBJ) $(JPEGLIBOBJ) $(LIBPNGOBJ) $(LIBAESGM) \
	$(BZIP2OBJ) $(EXTRAOBJ)

# ---------------------------------------------------------------------------
# Compiler flags
# ---------------------------------------------------------------------------

CXX ?= clang++
CC ?= clang
AR ?= ar
RM ?= rm -f

CXXINCS = -I. -I../../include -Izlib -Ijpeglib -Ilibpng
CPPFLAGS += $(CXXINCS) -DIRRLICHT_EXPORTS=1

CXXFLAGS += -Wall -pipe -fno-exceptions -fno-rtti -fstrict-aliasing

ifndef NDEBUG
CXXFLAGS += -g -D_DEBUG
else
CXXFLAGS += -O3
endif

ifdef PROFILE
CXXFLAGS += -pg
endif

CFLAGS += -O3 -DPNG_THREAD_UNSAFE_OK -DPNG_NO_MMX_CODE -DPNG_NO_MNG_FEATURES

sharedlib sharedlib_osx: CXXFLAGS += -fPIC
sharedlib sharedlib_osx: CFLAGS += -fPIC

# ---------------------------------------------------------------------------
# Platform settings
# ---------------------------------------------------------------------------

STATIC_LIB = libIrrlicht.a
LIB_PATH = ../../lib/$(SYSTEM)
INSTALL_DIR = /usr/local/lib

# Linux settings.
staticlib sharedlib install: SYSTEM = Linux
sharedlib install: SHARED_LIB = libIrrlicht.so
sharedlib: LDFLAGS += -L/usr/X11R6/lib -lGL -lXxf86vm
staticlib sharedlib: CXXINCS += -I/usr/X11R6/include

# macOS settings.
staticlib_osx sharedlib_osx install_osx: SYSTEM = MacOSX
sharedlib_osx install_osx: SHARED_LIB = libIrrlicht.dylib

staticlib_osx sharedlib_osx: CPPFLAGS += -IMacOSX -D_IRR_COMPILE_WITH_OSX_DEVICE_ -D_IRR_COMPILE_WITH_OPENGL_
sharedlib_osx sharedlib_osx: CXXINCS += -IMacOSX

# Native macOS frameworks. OpenGL is deprecated by Apple but remains available
# for Irrlicht 1.8.5 compatibility.
sharedlib_osx: LDFLAGS += -framework AppKit -framework Foundation -framework OpenGL -framework IOKit

# Windows settings.
IRRLICHT_DLL := ../../bin/Win32-gcc/Irrlicht.dll
sharedlib_win32 staticlib_win32: SYSTEM = Win32-gcc
sharedlib_win32: LDFLAGS += -lgdi32 -lopengl32 -ld3dx9d -lwinmm -Wl,--add-stdcall-alias
sharedlib_win32 staticlib_win32: CPPFLAGS += -DNO_IRR_COMPILE_WITH_DIRECTINPUT_JOYSTICK_ -DNO_IRR_COMPILE_WITH_DIRECT3D_9_ -DIRR_COMPILE_WITH_DX9_DEV_PACK -D__GNUWIN32__ -D_WIN32 -DWIN32 -D_WINDOWS -D_MBCS -D_USRDLL
staticlib_win32: CPPFLAGS += -D_IRR_STATIC_LIB_

VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_RELEASE)
SHARED_FULLNAME = $(SHARED_LIB).$(VERSION)
SONAME = $(SHARED_LIB).$(VERSION_MAJOR).$(VERSION_MINOR)

# ---------------------------------------------------------------------------
# Build targets
# ---------------------------------------------------------------------------

all linux: staticlib

sharedlib: $(LINKOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -shared -Wl,-soname,$(SONAME) -o $(SHARED_FULLNAME) $^ $(LDFLAGS)
	mkdir -p $(LIB_PATH)
	cp $(SHARED_FULLNAME) $(LIB_PATH)

$(STATIC_LIB): $(LINKOBJ)
	$(AR) rcs $@ $^

staticlib: $(STATIC_LIB)
	mkdir -p $(LIB_PATH)
	cp $(STATIC_LIB) $(LIB_PATH)

# macOS static library uses a separate object list so Linux/Windows sources
# are never pulled into this build.
staticlib_osx: libIrrlicht-osx.a
	mkdir -p $(LIB_PATH)
	cp libIrrlicht-osx.a $(LIB_PATH)/libIrrlicht.a

libIrrlicht-osx.a: $(LINKOBJ_OSX)
	$(AR) rcs $@ $^

sharedlib_osx: $(LINKOBJ_OSX)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -dynamiclib -Wl,-install_name,$(SONAME) -o $(SHARED_FULLNAME) $^ $(LDFLAGS)
	mkdir -p $(LIB_PATH)
	cp $(SHARED_FULLNAME) $(LIB_PATH)

all_win32 win32: sharedlib_win32

sharedlib_win32: $(IRRLICHT_DLL)

../../bin/Win32-gcc/Irrlicht.dll: $(LINKOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -shared -o $@ $^ $(LDFLAGS) -Wl,--out-implib,../../lib/Win32-gcc/$(STATIC_LIB)

staticlib_win32: $(STATIC_LIB)
	mkdir -p $(LIB_PATH)
	cp $(STATIC_LIB) $(LIB_PATH)

install install_osx:
	$(RM) -r $(INSTALL_DIR)/../include/irrlicht
	mkdir -p $(INSTALL_DIR)/../include/irrlicht
	cp ../../include/*.h $(INSTALL_DIR)/../include/irrlicht/
	cp $(LIB_PATH)/$(SHARED_FULLNAME) $(INSTALL_DIR)
	cd $(INSTALL_DIR) && ln -sf $(SHARED_FULLNAME) $(SONAME)
	cd $(INSTALL_DIR) && ln -sf $(SONAME) $(SHARED_LIB)

TAGS:
	ctags *.cpp ../../include/*.h *.h

# ---------------------------------------------------------------------------
# Compilation rules
# ---------------------------------------------------------------------------

# OpenGL driver references AppKit types on macOS, so compile as Objective-C++.
COpenGLDriver.o: COpenGLDriver.cpp
	$(CXX) -x objective-c++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

COpenGLDriver.d: COpenGLDriver.cpp
	$(CXX) -x objective-c++ $(CPPFLAGS) -MM -MF $@ $<


# Compile Objective-C++ sources that include macOS AppKit/Foundation headers.
Irrlicht.o: Irrlicht.cpp
	$(CXX) -x objective-c++ $(CPPFLAGS) $(CXXFLAGS) $(CXXINCS) -c -o $@ $<

Irrlicht.d: Irrlicht.cpp
	$(CXX) -x objective-c++ $(CPPFLAGS) $(CXXINCS) -MM -MF $@ $<


# macOS sources may contain Objective-C syntax despite their .cpp extension.
MacOSX/%.o: MacOSX/%.cpp
	$(CXX) -x objective-c++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

MacOSX/%.d: MacOSX/%.cpp
	$(CXX) -x objective-c++ $(CPPFLAGS) -MM -MF $@ $<

%.d: %.cpp
	$(CXX) $(CPPFLAGS) -MM -MF $@ $<

%.d: %.c
	$(CC) $(CPPFLAGS) -MM -MF $@ $<

%.o: %.mm
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

ifneq ($(MAKECMDGOALS),clean)
-include $(LINKOBJ:.o=.d)
-include $(LINKOBJ_OSX:.o=.d)
endif

help:
	@echo "Available Irrlicht targets:"
	@echo "  staticlib       Build the Linux static library"
	@echo "  sharedlib       Build the Linux shared library"
	@echo "  staticlib_osx   Build the macOS static library"
	@echo "  sharedlib_osx   Build the macOS dynamic library"
	@echo "  clean           Remove build outputs"

clean:
	$(RM) $(LINKOBJ) $(LINKOBJ:.o=.d)
	$(RM) $(LINKOBJ_OSX) $(LINKOBJ_OSX:.o=.d)
	$(RM) $(SHARED_FULLNAME) $(STATIC_LIB) libIrrlicht-osx.a

.PHONY: all linux staticlib sharedlib staticlib_osx sharedlib_osx \
	all_win32 win32 sharedlib_win32 staticlib_win32 help install install_osx clean
