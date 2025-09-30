#include <stdio.h>
#include <igraph.h>

#include "com_jni_PageRank.h"

#define EDGES       10

#ifdef INPUT_TEST
#define INPUT 10
#elif defined(INPUT_SMALL)
#define INPUT 10000
#else
#define INPUT 100000
#endif /* INPUT_TEST */

JNIEXPORT void JNICALL Java_com_jni_PageRank_pagerank(JNIEnv *env, jobject obj) {
        igraph_t graph;
        igraph_vector_t res;
        igraph_arpack_options_t arpack_opts;

        igraph_rng_seed(igraph_rng_default(), 42);
        igraph_arpack_options_init(&arpack_opts);
        igraph_vector_init(&res, 0);

        igraph_barabasi_game(&graph, 1000, 1, EDGES, NULL, 1, 0, IGRAPH_DIRECTED, IGRAPH_BARABASI_PSUMTREE, NULL);
        igraph_pagerank(&graph, IGRAPH_PAGERANK_ALGO_PRPACK, &res, NULL, igraph_vss_all(), IGRAPH_DIRECTED, 0.85, NULL, NULL);

        igraph_destroy(&graph);
        igraph_vector_destroy(&res);
}
