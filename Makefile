# 
# 1. General Compiler Settings
#
CXX       = g++
CXXFLAGS  = -std=c++14 -Wall -Wextra -Wcast-qual -Wno-unused-function -Wno-sign-compare -Wno-unused-value -Wno-unused-label -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-type-limits -Wno-vla -Wno-strict-aliasing -fno-exceptions -fno-rtti -pedantic -Wno-long-long -msse4.2 -mbmi -mbmi2 -D__STDC_CONSTANT_MACROS -DHAVE_SSE2=1 -DDSFMT_MEXP=521 -DLONGDOUBLE

INCLUDES  = -I../../lib/Box2D/Box2D_v2.3.0/Box2D/
LIBRARIES = ../../lib/Box2D/Box2D_v2.3.0/Box2D/Build/Box2D/libBox2D.a -lpthread

ifeq ($(OS),Windows_NT)
  # for Windows
  LIBRARIES += -IC:/x86_64-w64-mingw32/include/  -LC:/x86_64-w64-mingw32/lib/ -llibBox2D
else
  UNAME = \${shell uname}

  ifeq ($(UNAME),Linux)
    # for Linux
  endif

  ifeq ($(UNAME),Darwin)
    # for MacOSX
  endif
endif

#
# 2. Target Specific Settings
#
ifeq ($(TARGET),release)
	CXXFLAGS += -Ofast -DNDEBUG
        output_dir := out/release/
endif
ifeq ($(TARGET),debug)
	CXXFLAGS += -O0 -g -ggdb -D_GLIBCXX_DEBUG
        output_dir := out/debug/
endif
ifeq ($(TARGET),default)
	CXXFLAGS += -Ofast -g -ggdb -fno-fast-math
        output_dir := out/default/
endif
ifeq ($(TARGET),wrelease)
	CXXFLAGS += -Ofast -DNDEBUG
        output_dir := out/wrelease/
        LIBRARIES += -lwsock32 -lws2_32
endif

#
# 2. Default Settings (applied if there is no target-specific settings)
#
sources      ?= $(shell ls -R c/*.cc)
sources_dir  ?= c/
objects      ?=
directories  ?= $(output_dir)

#
# 4. Public Targets
#
default release debug development profile test coverage:
	$(MAKE) TARGET=$@ client_mcts_kr preparation jiritsu_test server server_gat dc_test logic_test ayumu_test image_log_maker table_generator variance_test shot_test client policy_client engine policy_engine dcl_converter dcl_counter simulator_test error_test interpolation_test eval_test policy_learner eval_learner client_pure client_pure_policy client_mcts client_mcts_policy client_mcts_policy_kr client_mcts_policy_go

run-coverage: coverage
	out/coverage --gtest_output=xml

clean:
	rm -rf out/*

scaffold:
	mkdir -p out test out/data doc lib obj resource

#
# 5. Private Targets
#
preparation $(directories):
	mkdir -p $(directories)

server :
	$(CXX) $(CXXFLAGS) -o $(output_dir)server $(sources_dir)server/server.cpp $(sources_dir)server/CurlingSimulator.cpp $(LIBRARIES) $(INCLUDES)

server_gat :
	$(CXX) $(CXXFLAGS) -o $(output_dir)server_gat $(sources_dir)server/server.cpp $(sources_dir)server/CurlingSimulator.cpp -DRULE_ERROR_GAT $(LIBRARIES) $(INCLUDES)

client :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client $(sources_dir)client.cc -DDEFAULT_SETTINGS $(LIBRARIES) $(INCLUDES)

client_pure :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_pure $(sources_dir)client.cc $(LIBRARIES) $(INCLUDES)

client_pure_policy :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_pure_policy $(sources_dir)client.cc -DUSE_POLICY_SCORE -DUSE_HANDMADE_MOVES $(LIBRARIES) $(INCLUDES)

client_mcts :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_mcts $(sources_dir)client.cc -DUSE_MCTS $(LIBRARIES) $(INCLUDES)

client_mcts_policy :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_mcts_policy $(sources_dir)client.cc -DUSE_MCTS -DUSE_POLICY_SCORE -DUSE_HANDMADE_MOVES $(LIBRARIES) $(INCLUDES)

client_mcts_kr :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_mcts_kr $(sources_dir)client.cc -DUSE_MCTS -DUSE_KERNEL_REGRESSION $(LIBRARIES) $(INCLUDES)

client_mcts_policy_kr :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_mcts_policy_kr $(sources_dir)client.cc -DUSE_MCTS -DUSE_POLICY_SCORE -DUSE_HANDMADE_MOVES -DUSE_KERNEL_REGRESSION $(LIBRARIES) $(INCLUDES)

client_mcts_go :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_mcts_go $(sources_dir)client.cc -DUSE_MCTS -DUSE_GAUSSIAN_OPTIMIZATION $(LIBRARIES) $(INCLUDES)

client_mcts_policy_go :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client_mcts_policy_go $(sources_dir)client.cc -DUSE_MCTS -DUSE_POLICY_SCORE -DUSE_HANDMADE_MOVES -DUSE_GAUSSIAN_OPTIMIZATION $(LIBRARIES) $(INCLUDES)

engine :
	$(CXX) $(CXXFLAGS) -o $(output_dir)engine $(sources_dir)client.cc -DENGINE $(LIBRARIES) $(INCLUDES)

policy_client :
	$(CXX) $(CXXFLAGS) -o $(output_dir)policy_client $(sources_dir)client.cc -DPOLICY_ONLY $(LIBRARIES) $(INCLUDES)

policy_engine :
	$(CXX) $(CXXFLAGS) -o $(output_dir)policy_engine $(sources_dir)client.cc -DPOLICY_ONLY -DENGINE $(LIBRARIES) $(INCLUDES)

dcl_converter: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)dcl_converter $(sources_dir)log/dcl_converter.cc $(LIBRARIES) $(INCLUDES)

dcl_counter: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)dcl_counter $(sources_dir)log/dcl_counter.cc $(LIBRARIES) $(INCLUDES)

eval_learner: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)eval_learner $(sources_dir)ayumu/evlearner.cc $(LIBRARIES) $(INCLUDES)

policy_learner: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)policy_learner $(sources_dir)ayumu/pglearner.cc $(LIBRARIES) $(INCLUDES)

dc_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)dc_test $(sources_dir)test/dc_test.cc $(LIBRARIES) $(INCLUDES)

simulator_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)simulator_test $(sources_dir)test/simulator_test.cc $(LIBRARIES) $(INCLUDES)

interpolation_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)interpolation_test $(sources_dir)test/interpolation_test.cc $(LIBRARIES) $(INCLUDES)

shot_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)shot_test $(sources_dir)test/shot_test.cc $(LIBRARIES) $(INCLUDES)

error_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)error_test $(sources_dir)test/error_test.cc $(LIBRARIES) $(INCLUDES)

eval_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)eval_test $(sources_dir)test/eval_test.cc $(LIBRARIES) $(INCLUDES)

solver_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)solver_test $(sources_dir)test/solver_test.cc $(LIBRARIES) $(INCLUDES)

variance_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)variance_test $(sources_dir)test/variance_test.cc $(LIBRARIES) $(INCLUDES)

logic_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)logic_test $(sources_dir)test/logic_test.cc $(LIBRARIES) $(INCLUDES)

ayumu_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)ayumu_test $(sources_dir)test/ayumu_test.cc $(LIBRARIES) $(INCLUDES)

table_generator: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)table_generator $(sources_dir)test/table_generator.cc $(LIBRARIES) $(INCLUDES)

image_log_maker: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)image_log_maker $(sources_dir)test/image_log_maker.cc $(LIBRARIES) $(INCLUDES)

jiritsu_test: $(directories) $(objects)
	$(CXX) $(CXXFLAGS) -o $(output_dir)jiritsu_test $(sources_dir)test/jiritsu_test.cc $(LIBRARIES) $(INCLUDES)