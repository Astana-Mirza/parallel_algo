#ifndef THREAD_SAFE_QUEUE_INL
#define THREAD_SAFE_QUEUE_INL

#include <cassert>

template < typename T >
ThreadSafeQueue< T >::ThreadSafeQueue( size_t producer_count ):
     producer_count_{ producer_count }
{}


template < typename T >
void ThreadSafeQueue< T >::remove_all_producers()
{
     std::unique_lock< std::mutex > lock{ mutex_ };
     producer_count_ = 0;
     cond_.notify_all();
}


template < typename T >
void ThreadSafeQueue< T >::remove_producer()
{
     std::unique_lock< std::mutex > lock{ mutex_ };
     if ( producer_count_ > 0 && --producer_count_ == 0 )
     {
          cond_.notify_all();
     }
}


template < typename T >
void ThreadSafeQueue< T >::push( T&& elem )
{
     std::unique_lock< std::mutex > lock{ mutex_ };
     queue_.push( elem );
     cond_.notify_one();
}


template < typename T >
bool ThreadSafeQueue< T >::process( const std::function< void( const T& ) >& func )
{
     std::pair< bool, T > res;
     res.first = false;
     {
          std::unique_lock< std::mutex > lock{ mutex_ };
          cond_.wait( lock, [ this ](){ return !queue_.empty() || producer_count_ == 0; } );
          if ( !queue_.empty() )
          {
               res = std::make_pair( true, queue_.front() );
               queue_.pop();
          }
     }
     if ( res.first )
     {
          func( res.second );
     }
     return res.first;
}

#endif // THREAD_SAFE_QUEUE_INL
