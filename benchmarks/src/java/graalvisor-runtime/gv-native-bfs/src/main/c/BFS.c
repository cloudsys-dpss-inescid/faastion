#include <stdio.h>
#include <igraph.h>

#include "com_jni_BFS.h"

#define EDGES       10

#ifdef INPUT_TEST
#define INPUT 10
#elif defined(INPUT_SMALL)
#define INPUT 10000
#else
#define INPUT 100000
#endif /* INPUT_TEST */

JNIEXPORT void JNICALL Java_com_jni_BFS_bfs(JNIEnv *env, jobject obj) {
        igraph_t graph;
        igraph_rng_seed(igraph_rng_default(), 42);
        igraph_barabasi_game(&graph, 100, 1, EDGES, NULL, 1, 0, IGRAPH_DIRECTED, IGRAPH_BARABASI_PSUMTREE, NULL);

        igraph_bfs(&graph, 0, NULL, IGRAPH_OUT,
                1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        igraph_destroy(&graph);
}
