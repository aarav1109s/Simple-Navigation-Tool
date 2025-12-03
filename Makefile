# ========================================================================================= #
#  C++ Project Makefile      														        #
#  Angelo Kyrilov                      														#
#  University of California, Merced    												   		#
# ========================================================================================= #

# ===================================== PROJECT CONFIG ==================================== #

SRC_DIR = src
HEADERS_DIR = inc
TEST_DIR = test

OBJ_DIR = objects
BIN_DIR = bin

APP = app
MAIN = main
TEST = test

# =================================== COMPILER SETTINGS =================================== #

CXX = g++

ifeq ($(filter autograde,$(MAKECMDGOALS)),autograde)
CXXFLAGS = -Wall -Wextra -Werror -I$(HEADERS_DIR)
else
CXXFLAGS = -Wall -I$(HEADERS_DIR)
endif


LDFLAGS = -lfltk_images -lpng -lz -lfltk_gl -lGLU -lGL -lfltk -lXrender \
          -lXext -lXft -lfontconfig -lpthread -ldl -lm -lX11

MAKEFLAGS += --no-print-directory

# ==================================== BANNED HEADERS ===================================== #

NOT_ALLOWED = stl_vector|stl_list|stl_deque|std_stack|stl_queue|stl_set|stl_map|unordered_map|unordered_set|stl_algo.h
EXCLUDE = src/main.cpp src/Application.cpp

# ======================================== TARGETS ======================================== #

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OUT = $(BIN_DIR)/$(APP)

TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ = $(TEST_SRC:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_OUT = $(BIN_DIR)/$(TEST)

HEADERS = $(wildcard $(HEADERS_DIR)/*.h)

all: $(OUT)

$(OUT): $(OBJ) $(BIN_DIR) check-banned-headers
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(OUT) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(OBJ_DIR) $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)


run: $(OUT)
	@clear
	@$(BIN_DIR)/$(APP)

test: $(OBJ) $(TEST_OBJ) $(BIN_DIR) check-banned-headers
	$(CXX) $(CXXFLAGS) $(filter-out $(OBJ_DIR)/$(MAIN).o, $(OBJ)) $(TEST_OBJ) -o $(TEST_OUT) $(LDFLAGS)
	@clear
	@$(BIN_DIR)/$(TEST) --output=color || true

autograde: clean $(OBJ) $(TEST_OBJ) $(BIN_DIR) check-banned-headers
	@$(CXX) $(CXXFLAGS) $(filter-out $(OBJ_DIR)/$(MAIN).o, $(OBJ)) $(TEST_OBJ) -o $(TEST_OUT) $(LDFLAGS)
	@xvfb-run $(BIN_DIR)/$(TEST) || true

$(OBJ_DIR)/$(TEST).o: $(TEST_DIR)/$(TEST).cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $(TEST_DIR)/$(TEST).cpp -o $(OBJ_DIR)/$(TEST).o

clean:
	@rm -f $(BIN_DIR)/$(APP) $(OBJ) $(BIN_DIR)/$(TEST) $(TEST_OBJ)
	@rmdir $(BIN_DIR) $(OBJ_DIR) 2> /dev/null || true
	@echo Project folder clean

check-banned-headers:
	@echo "🔍  Scanning files for banned headers..."
	@violations=0; \
	for file in $(SRC); do \
		skip=0; \
		for excluded in $(EXCLUDE); do \
			if [ "$$file" = "$$excluded" ]; then \
				skip=1; \
				break; \
			fi; \
		done; \
		if [ $$skip -eq 1 ]; then \
			echo "⚠️  Skipping excluded file: $$file"; \
			continue; \
		fi; \
		echo "➡️  Checking $$file..."; \
		if [ -n "$(NOT_ALLOWED)" ]; then \
			deps=$$($(CXX) $(CXXFLAGS) -M $$file | grep -E '$(NOT_ALLOWED)'); \
		else \
			deps=""; \
		fi; \
		if [ -n "$$deps" ]; then \
			echo "❌  Banned headers found in $$file:"; \
			echo "$$deps"; \
			violations=1; \
		else \
			echo "✅. No banned headers in $$file."; \
		fi; \
	done; \
	if [ $$violations -ne 0 ]; then \
		echo ""; \
		echo "🚫  Build failed: Banned headers detected."; \
		exit 1; \
	else \
		echo ""; \
		echo "🎉  All files are clean!"; \
	fi

pull:
	@if git rev-parse --is-inside-work-tree > /dev/null 2>&1; then \
		if [ -n "$$(git status --porcelain)" ]; then \
			printf "⚠️  \033[31m\033[1mWARNING:\033[0m This action will discard your changes.\n"; \
			read -p "Do you want to continue? [y/N]: " ans && [ "$$ans" = y ] && { \
				git reset --hard && \
				git clean -fdx && \
				git pull --ff-only; \
			} || echo "Aborted."; \
		else \
			git pull --ff-only; \
		fi \
	else \
		printf "🚫  \033[31m\033[1mERROR:\033[0m Not a git repository.\n"; \
	fi

.PHONY: run pull test autograde clean check-banned-headers
