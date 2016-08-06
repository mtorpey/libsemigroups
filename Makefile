SOURCES = $(wildcard *.cc)
HEADERS = $(wildcard *.h)

OBJECTS = ${SOURCES:.cc=.o}
CXXFLAGS = -I. -fPIC -Wno-c++11-extensions -std=c++11

COMMON_DOC_FLAGS = --report --merge docs --output html $(SOURCES) $(HEADERS)

doc:
	@echo "Generating documentation..."; \
	cldoc generate $(CXXFLAGS) -- $(COMMON_DOC_FLAGS)

static-doc:
	@echo "Generating static documentation..."; \
	cldoc generate $(CXXFLAGS) -- --static $(COMMON_DOC_FLAGS)
	@echo "Fixing some bugs in cldoc..."; \
	python docs/cldoc-fix

serve:
	cldoc serve html
