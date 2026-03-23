# FunnyCraft Makefile

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
WXFLAGS = $(shell wx-config --cxxflags)
WXLIBS = $(shell wx-config --libs)

# Directories
SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/funnycraft

# Source files
SOURCES = $(SRCDIR)/main.cpp \
          $(SRCDIR)/MainFrame.cpp \
          $(SRCDIR)/AccountManager.cpp \
          $(SRCDIR)/InstanceManager.cpp \
          $(SRCDIR)/MinecraftLauncher.cpp \
          $(SRCDIR)/AccountDialog.cpp \
          $(SRCDIR)/InstanceDialog.cpp \
          $(SRCDIR)/DownloadManager.cpp \
          $(SRCDIR)/PathUtils.cpp \
          $(SRCDIR)/ProgressDialog.cpp \
          $(SRCDIR)/InstanceSettingsDialog.cpp \
          $(SRCDIR)/SettingsDialog.cpp

# Object files
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Default target
all: $(BUILDDIR) $(TARGET)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Link executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(WXLIBS) -lstdc++fs
	@echo "Build complete: $(TARGET)"

# Compile source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(WXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)
	@echo "Clean complete"

# Install (optional)
install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)/usr/bin/funnycraft
	@echo "Installed to $(DESTDIR)/usr/bin/funnycraft"

# Run the application
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS = -std=c++17 -Wall -Wextra -g -O0 -DDEBUG
debug: clean all

# Release build
release: CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -DNDEBUG
release: clean all

.PHONY: all clean install run debug release
