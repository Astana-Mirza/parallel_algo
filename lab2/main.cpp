#include <matrix.h>
#include <utils.h>
#include <thread_safe_queue.h>

#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <csignal>
#include <iostream>

namespace
{

using Task = std::pair< Matrix, Matrix >;
using TaskQueue = ThreadSafeQueue< Task >;
using MatrixQueue = ThreadSafeQueue< Matrix >;

volatile std::sig_atomic_t g_stop_threads{ false };


void on_sigint( int )
{
     g_stop_threads = true;
}


void produce( size_t tasks_count, size_t size, TaskQueue& tasks )
{
     bool infinite = ( tasks_count == 0 );
     while ( !g_stop_threads && ( infinite || tasks_count > 0 ) )
     {
          tasks.push( { Matrix::generate( size, size ), Matrix::generate( size, size ) } );
          if ( !infinite )
          {
               tasks_count--;
          }
     }
     tasks.remove_producer();
}


void consume( TaskQueue& tasks, MatrixQueue& matrices )
{
     auto processor = [ &matrices ]( const Task& task )
     {
          matrices.push( task.first * task.second );
     };
     while ( !g_stop_threads && tasks.process( processor ) );
     matrices.remove_producer();
}


void output( MatrixQueue& matrices, std::ostream& out )
{
     auto processor = [ &matrices, &out ]( const Matrix& matr )
     {
          out << "RESULT\n" << matr << '\n';
     };
     while ( matrices.process( processor ) );
}


} // anonymous namespace

int main( int argc, char *argv[] )
{
     size_t task_count = 0;
     size_t matrix_size = 3;
     size_t producer_count = 1;
     size_t consumer_count = 1;

     if ( argc != 5
          || !str_to_size( argv[ 1 ], producer_count )
          || !str_to_size( argv[ 2 ], consumer_count )
          || !str_to_size( argv[ 3 ], task_count )
          || !str_to_size( argv[ 4 ], matrix_size ) )
     {
          std::cerr << "Usage: " << argv[ 0 ] << " <producer_count> <consumer_count> <task_count> <matrix_size>\n";
          return EXIT_FAILURE;
     }

     std::srand( std::time( nullptr ) );
     std::signal( SIGINT, on_sigint );

     TaskQueue tasks( producer_count );
     MatrixQueue matrices( consumer_count );

     std::vector< std::thread > producers;
     std::vector< std::thread > consumers;
     producers.reserve( producer_count );
     consumers.reserve( consumer_count );

     for ( size_t i = 0; i < producer_count; i++ )
     {
          producers.emplace_back( produce, task_count, matrix_size, std::ref( tasks ) );
     }
     for ( size_t i = 0; i < consumer_count; i++ )
     {
          consumers.emplace_back( consume, std::ref( tasks ), std::ref( matrices ) );
     }
     std::thread printer( output, std::ref( matrices ), std::ref( std::cout ) );


     for ( size_t i = 0; i < producer_count; i++ )
     {
          producers[ i ].join();
     }
     for ( size_t i = 0; i < consumer_count; i++ )
     {
          consumers[ i ].join();
     }
     printer.join();

     return EXIT_SUCCESS;
}


