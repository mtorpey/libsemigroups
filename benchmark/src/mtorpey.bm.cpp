//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2017 Michael Torpey
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// This file contains some benchmarks for libsemigroups/src/cong.cc

#include <benchmark/benchmark.h>
#include <libsemigroups/cong.h>

using namespace libsemigroups;

// Number of repetitions
u_int32_t reps = 1;

// TODO: Add BM_mtorpey_ to the start of all the benchmark names

////
//T/ Full_PBR_universal: bad for prefill, great for normal TC
////
static void Full_PBR_universal_force_prefill(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<Element*> gens = {
        new PBR(new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{}, {2}, {1}, {0, 3}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{0, 3}, {2}, {1}, {}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{1, 2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1, 3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {1}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {0, 1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {1}, {0}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2, 3}, {0}, {1}}))};

    Semigroup S = Semigroup(gens);
    S.set_report(false);
    really_delete_cont(gens);

    std::vector<relation_t> extra(
        {relation_t({7, 10, 9, 3, 6, 9, 4, 7, 9, 10},
                    {9, 3, 6, 6, 10, 9, 4, 7}),
         relation_t({0}, {1}),
         relation_t({0}, {2}),
         relation_t({0}, {3}),
         relation_t({0}, {4}),
         relation_t({0}, {5}),
         relation_t({0}, {6}),
         relation_t({0}, {7}),
         relation_t({0}, {8}),
         relation_t({0}, {9}),
         relation_t({0}, {10}),
         relation_t({8, 7, 5, 8, 9, 8}, {6, 3, 8, 6, 1, 2, 4})});
    Congruence cong("twosided", &S, extra);
    cong.set_report(false);
    cong.force_tc_prefill();

    auto start = std::chrono::high_resolution_clock::now();
    cong.nr_classes();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(Full_PBR_universal_force_prefill)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

static void Full_PBR_universal_force_tc(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<Element*> gens = {
        new PBR(new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{}, {2}, {1}, {0, 3}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{0, 3}, {2}, {1}, {}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{1, 2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1, 3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {1}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {0, 1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {1}, {0}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2, 3}, {0}, {1}}))};

    Semigroup S = Semigroup(gens);
    S.set_report(false);
    really_delete_cont(gens);

    std::vector<relation_t> extra(
        {relation_t({7, 10, 9, 3, 6, 9, 4, 7, 9, 10},
                    {9, 3, 6, 6, 10, 9, 4, 7}),
         relation_t({0}, {1}),
         relation_t({0}, {2}),
         relation_t({0}, {3}),
         relation_t({0}, {4}),
         relation_t({0}, {5}),
         relation_t({0}, {6}),
         relation_t({0}, {7}),
         relation_t({0}, {8}),
         relation_t({0}, {9}),
         relation_t({0}, {10}),
         relation_t({8, 7, 5, 8, 9, 8}, {6, 3, 8, 6, 1, 2, 4})});
    Congruence cong("twosided", &S, extra);
    cong.set_report(false);
    cong.force_tc();

    auto start = std::chrono::high_resolution_clock::now();
    cong.nr_classes();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(Full_PBR_universal_force_tc)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

////
//T/ Full_PBR_manyclasses: great for prefill, bad for normal TC
////

static void Full_PBR_manyclasses_force_prefill(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<Element*> gens = {
        new PBR(new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{}, {2}, {1}, {0, 3}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{0, 3}, {2}, {1}, {}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{1, 2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1, 3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {1}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {0, 1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {1}, {0}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2, 3}, {0}, {1}}))};

    Semigroup S = Semigroup(gens);
    S.set_report(false);
    really_delete_cont(gens);

    std::vector<relation_t> extra(
        {relation_t({7, 10, 9, 3, 6, 9, 4, 7, 9, 10},
                    {9, 3, 6, 6, 10, 9, 4, 7}),
         relation_t({8, 7, 5, 8, 9, 8}, {6, 3, 8, 6, 1, 2, 4})});
    Congruence cong("twosided", &S, extra);
    cong.set_report(false);
    cong.force_tc_prefill();

    auto start = std::chrono::high_resolution_clock::now();
    cong.nr_classes();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(Full_PBR_manyclasses_force_prefill)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

static void Full_PBR_manyclasses_force_tc(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<Element*> gens = {
        new PBR(new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{}, {2}, {1}, {0, 3}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{0, 3}, {2}, {1}, {}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{1, 2}, {3}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{2}, {3}, {0}, {1, 3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {1}, {0}, {1}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {0, 1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {1}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {0}, {3}})),
        new PBR(new std::vector<std::vector<u_int32_t>>({{3}, {2}, {1}, {0}})),
        new PBR(
            new std::vector<std::vector<u_int32_t>>({{3}, {2, 3}, {0}, {1}}))};

    Semigroup S = Semigroup(gens);
    S.set_report(false);
    really_delete_cont(gens);

    std::vector<relation_t> extra(
        {relation_t({7, 10, 9, 3, 6, 9, 4, 7, 9, 10},
                    {9, 3, 6, 6, 10, 9, 4, 7}),
         relation_t({8, 7, 5, 8, 9, 8}, {6, 3, 8, 6, 1, 2, 4})});
    Congruence cong("twosided", &S, extra);
    cong.set_report(false);
    cong.force_tc();

    auto start = std::chrono::high_resolution_clock::now();
    cong.nr_classes();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(Full_PBR_manyclasses_force_tc)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

////
//T/ KBP 12: only works with KBP and KBFP?
////

static void KBP_12_allow_all(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<relation_t> rels
      = {relation_t({0, 0, 0}, {0}), relation_t({0, 1}, {1, 0})};
    std::vector<relation_t> extra = {relation_t({0}, {0, 0})};
    Congruence              cong("twosided", 2, rels, extra);
    cong.set_report(false);

    word_t x = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    word_t y = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    auto start = std::chrono::high_resolution_clock::now();
    cong.test_equals(x, y);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(KBP_12_allow_all)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

static void KBP_12_force_kbfp(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<relation_t> rels
      = {relation_t({0, 0, 0}, {0}), relation_t({0, 1}, {1, 0})};
    std::vector<relation_t> extra = {relation_t({0}, {0, 0})};
    Congruence              cong("twosided", 2, rels, extra);
    cong.force_kbfp();
    cong.set_report(false);

    word_t x = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    word_t y = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    auto start = std::chrono::high_resolution_clock::now();
    cong.test_equals(x, y);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(KBP_12_force_kbfp)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

static void KBP_12_force_kbp(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<relation_t> rels
      = {relation_t({0, 0, 0}, {0}), relation_t({0, 1}, {1, 0})};
    std::vector<relation_t> extra = {relation_t({0}, {0, 0})};
    Congruence              cong("twosided", 2, rels, extra);
    cong.force_kbp();
    cong.set_report(false);

    word_t x = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    word_t y = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    auto start = std::chrono::high_resolution_clock::now();
    cong.test_equals(x, y);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(KBP_12_force_kbp)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

////
//T/ KBP 08: only works with KBP?
////

static void KBP_08_force_kbp(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<relation_t> rels = {
      relation_t({1, 1, 1, 1, 1, 1, 1}, {1}),
      relation_t({2, 2, 2, 2, 2}, {2}),
      relation_t({1, 2, 2, 1, 0}, {1, 2, 2, 1}),
      relation_t({1, 2, 2, 1, 2}, {1, 2, 2, 1}),
      relation_t({1, 1, 2, 1, 2, 0}, {1, 1, 2, 1, 2}),
      relation_t({1, 1, 2, 1, 2, 1}, {1, 1, 2, 1, 2}),
    };

    std::vector<relation_t> extra = {relation_t({1, 2, 2, 1}, {1, 1, 2, 1, 2})};
    Congruence              cong("right", 3, rels, extra);
    cong.force_kbp();
    cong.set_report(false);

    auto start = std::chrono::high_resolution_clock::now();
    cong.word_to_class_index({1, 2, 2, 1}) == cong.word_to_class_index({1, 1, 2, 1, 2});
    Partition<word_t>* ntc = cong.nontrivial_classes();
    delete ntc;
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds
        = std::chrono::duration_cast<std::chrono::duration<double>>(end
                                                                    - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(KBP_08_force_kbp)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(reps)
    ->UseManualTime();

BENCHMARK_MAIN();
