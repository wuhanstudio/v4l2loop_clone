CXXFLAGS   = -Os `pkg-config --cflags opencv4`
LDLIBS     = `pkg-config --libs opencv4`
TARGET     = v4l2loop_clone

all: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET)
