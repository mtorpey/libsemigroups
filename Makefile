OBJ_DIR = test/bin
TEST_OBJ_DIR = test/bin/test
LOG_DIR = test/logs
TODAY = $(shell date "+%Y-%m-%d-%H-%M-%S")

SOURCES = $(wildcard *.cc)
HEADERS = $(wildcard *.h)
OBJECTS = $(SOURCES:%.cc=$(OBJ_DIR)/%.o)

TEST_SOURCES = $(wildcard test/*.cc)
TEST_OBJECTS = $(TEST_SOURCES:%.cc=$(OBJ_DIR)/%.o)

CXXFLAGS = -I. -Wall -Wextra -pedantic -Wno-c++11-extensions -std=c++11

ifdef COVERAGE
  CXXFLAGS += -O0 -g --coverage
  LDFLAGS += -O0 -g --coverage
else
  CXXFLAGS += -O2
endif 

ifneq ($(CXX),clang++)
  ifneq ($(CXX), c++) 
    CXXFLAGS += -pthread
   endif
endif

COMMON_DOC_FLAGS = --report --merge docs --output html $(SOURCES) $(HEADERS)

$(OBJ_DIR)/%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

error:
	@echo "Please choose one of the following: doc, serve, test,"
	@echo " or testclean"; \
	@exit 2
doc:
	@echo "Generating static documentation . . ."; \
	cldoc generate $(CXXFLAGS) -- --static $(COMMON_DOC_FLAGS)
	@echo "Fixing some bugs in cldoc . . ."; \
	python docs/cldoc-fix

serve:
	cldoc serve html

test: testdirs $(TEST_OBJECTS) $(OBJECTS)
	@echo "Building the test executable . . ."; \
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(TEST_OBJECTS) -o test/test $(LDFLAGS)
	@echo "Running the tests ("$(LOG_DIR)/$(TODAY).log") . . ."; \
	test/test -d yes --order lex --force-colour | tee -a $(LOG_DIR)/$(TODAY).log

testclean:
	rm -rf $(OBJ_DIR) test/test

testdirs:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(TEST_OBJ_DIR)
	mkdir -p $(LOG_DIR)

#testcov: $(TEST_OBJECTS) $(OBJECTS)
#	@echo "This does not work correctly!"; \
#	# TODO only make test/cov/clean if necessary
#	make testcovclean
#	make testclean
#	make test COVERAGE=true
#	lcov --no-external --capture --base-directory /Users/jdm/semigroups/src/semigroupsplusplus --directory /Users/jdm/semigroups/src/semigroupsplusplus/test --output-file test/lcov.info
#	genhtml test/lcov.info --output-directory test/lcov-out
#	open test/lcov-out/index.html
#
#testcovclean:
#	rm -rf test/lcov-out lcov.info
#	rm -f $(OBJ_DIR)/*.gcda $(OBJ_DIR)/*.gcno

.PHONY: test dirs
