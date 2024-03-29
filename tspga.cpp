/*
   File: tspga.cpp
   Authors: Sam Lebherz (slebher1), Andrew Sillers (andsil1)
   TSP Genetic Algorithm
   CMSC 491, Spring 2010
   
   Driver that constructs a Genetic Algorithm (GA) to 
   solve the Traveling Salesman Problem (TSP).
   
   The GA follows these high level steps:
   
      1) Construct a population.
      2) Genesis, initial evaluation.
      3) Evolve until termination conditions are satisfied:
         a) Reproduce
         b) Evaluate
         c) Merge
*/

#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include "population.h"
#include "individual.h"
#include <omp.h>

using namespace std;

#define EPSILON              0               // A percent (of the best path)
#define BEST_PATH            847             // Brute force tsp solution
#define MAX_ITERATIONS       10000           // Stop evolve loop when reached
#define MAX_STALE            1000            // Max consecutive iterations
                                             //    allowed without improvement
#define POP_SIZE             200             // Size of popultion. this is the 
                                             //    number of paths that Genesis
                                             //    will read in
#define ELITISM              0.20            // Percent of pop to preserve
#define MUTATION_RATE        0.30            // Chance a new indiv will mutate
#define INITIAL_PATHS_FNAME  "initial.dat"   // Paths for initial population
#define TSP_DATA_FNAME       "tsp.dat"       // TSP loaded from here

// Only one of these should ever be set to true at once.
#define PLOT_ITER            false
#define PLOT_FIT             false

bool terminate(int num_iterations, double highest_fitness, int stale_iterations);
void log(int num_iterations, Population* tsp_pop);

int main() {
   setbuf(stdout, NULL); // so when we print the plotting output it prints NOW
   srand(time(NULL));

   // Timing stuff
   struct timeval systime, systime2;
	double t1, t2, t3, t4, total_time, total_iter_time;
   total_iter_time = 0;

   // GA stuff
   int num_iterations = 0;
   double last_fitness = 0;
   int stale_iterations = 0;
   
   /* Start GA Timer */
   gettimeofday(&systime,NULL);
	t1 = systime.tv_sec + (systime.tv_usec/1000000.0);
   
   /*
      Setup for evolution loop:
         1) Construct a new population.
         2) Genesis event occurs.
         3) Evaluate initial population
   */
   Population tsp_pop(INITIAL_PATHS_FNAME, TSP_DATA_FNAME, POP_SIZE, ELITISM, MUTATION_RATE);
   tsp_pop.Genesis();
   tsp_pop.Evaluate();
   tsp_pop.Merge(true);
   
   /*
      Evolution - Do this until termination conditions are met:
         1) Reproduce
         2) Evaluate offspring.
         3) Merge current population and offspring.
         4) log dump, visualization update, etc...
   */

   while(!terminate(num_iterations, tsp_pop.Fittest().Raw_Fitness(),
                    stale_iterations)) {
      // Start Iteration Timer
      gettimeofday(&systime2,NULL);
      t3 = systime2.tv_sec + (systime2.tv_usec/1000000.0);
      
      tsp_pop.Reproduce();
      tsp_pop.Evaluate();
      tsp_pop.Merge();
      
      // End Iteration Timer
      gettimeofday(&systime2,NULL);
      t4 = systime2.tv_sec + (systime2.tv_usec/1000000.0);
      total_iter_time += (t4 - t3);
      
      num_iterations++;

      // if there was no improvement of the max, increase stale_iterations
      if(tsp_pop.Fittest().Raw_Fitness() == last_fitness) {
         stale_iterations++;
      }
      else {
         stale_iterations = 0;
         last_fitness = tsp_pop.Fittest().Raw_Fitness();
      }

      // output stats every 10 generations
      if(num_iterations % 10 == 0) {
         log(num_iterations, &tsp_pop);
      }
      
      // output plotting updates every generation
      if(PLOT_ITER || PLOT_FIT) {
         if(PLOT_ITER) {
            printf("%.4f\n", (t4 - t3));
         }
         else if (PLOT_FIT) {
            printf("%.0f %.1f %d\n", tsp_pop.Fittest().Raw_Fitness(), tsp_pop.Avg_Fitness(), BEST_PATH); 
         }
      }
   }
  
   /* End GA Timer */
   gettimeofday(&systime,NULL);
   t2 = systime.tv_sec + (systime.tv_usec/1000000.0);
   total_time = t2-t1;
   
   // Did we find the optimal path?
   if(tsp_pop.Fittest().Raw_Fitness() == BEST_PATH) {
     fprintf(stderr, "Best path (%d) found!\n", BEST_PATH);
   }
   else {
      fprintf(stderr, "Best path NOT found!\n");
   }
   
   fprintf(stderr, "GA Time: %.1lf (seconds)\n", total_time);
   fprintf(stderr, "Avg Iteration Time: %.4lf (seconds)\n", total_iter_time/(double)num_iterations);
   log(num_iterations, &tsp_pop);
   
   return 0;
}

/*
   Termination conditions - meeting any one is sufficient:
      1) Reached max number of iterations.
      2) Fittest individual's path is within epsilon of the best path.
*/
bool terminate(int num_iterations, double highest_fitness, int stale_iterations) {
   if(num_iterations == MAX_ITERATIONS) 
      return true;
   
   if((highest_fitness - BEST_PATH) <= (EPSILON * BEST_PATH))
     return true;

   if(stale_iterations == MAX_STALE)
      return true;
   
   return false;
}

/*
   Displays:
      Generation
      Average Fitness
      Fittest Individual
*/
void log(int num_iterations, Population* tsp_pop) {
   fprintf(stderr, "Generation: %d\n", num_iterations);
   fprintf(stderr, "Average Fitness: %.1f\n", tsp_pop->Avg_Fitness());
   fprintf(stderr, "Fittest Individual:\n");
   tsp_pop->Fittest().Print();
   fprintf(stderr, "\n\n");
}
