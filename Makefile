CXX = g++
# Добавляем флаги для генерации зависимостей
CXXFLAGS = -std=c++23 -O2 -g -Wall -Wextra -MMD -MP
LDFLAGS = -fsanitize=address,undefined -pthread

ASAN_LIB   := $(shell $(CXX) -print-file-name=libasan.so)

TARGET = my_engine.out
SRC_DIR = src
# Найдем все .cpp файлы в текущей директории
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

OBJS = $(SRCS:.cpp=.o)
# Создаем список .d файлов (файлов зависимостей)
DEPS = $(OBJS:.o=.d)

GPERF_HPP = $(SRC_DIR)/uci_lookup.hpp
GPERF_SRC = $(SRC_DIR)/uci_commands.gperf

GPERF = gperf
GPERF_FLAGS = -L C++ -C -t

ifeq ($(DEBUG),1)
  CXXFLAGS += -DDEBUG
endif

# ---- Правила сборки ----

all: $(TARGET)

# Правило линковки
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Правило для gperf
$(GPERF_HPP): $(GPERF_SRC)
	@echo "Generating $@ from $< (via cpp + gperf)..."
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -x c++ -std=c++23 -E -P $< | \
		$(GPERF) $(GPERF_FLAGS) > $@

# Правило компиляции
# Теперь зависит только от .cpp, Makefile и СГЕНЕРИРОВАННОГО .hpp
# Зависимости от других .h/.hpp будут добавлены через include
%.o: %.cpp $(GPERF_HPP) Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up..."
	# Удаляем также и .d файлы
	rm -f $(TARGET) $(OBJS) $(DEPS) $(GPERF_HPP) core *~

# Включаем сгенерированные файлы зависимостей
# Флаг '-' перед include означает, что make не будет выдавать ошибку,
# если файлы .d еще не существуют (например, при первой сборке или после clean)
-include $(DEPS)

test: $(TARGET)
	@echo "Running with ASan preloaded from $(ASAN_LIB)"
	LD_PRELOAD=$(ASAN_LIB) ./$(TARGET) < test

.PHONY: all clean test