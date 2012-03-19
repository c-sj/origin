// Copyright (c) 2008-2010 Kent State University
// Copyright (c) 2011-2012 Texas A&M University
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

#ifndef ORIGIN_RANDOM_HPP
#define ORIGIN_RANDOM_HPP

#include <random>
#include <string>

#include <origin/container_fwd.hpp>
#include <origin/concepts.hpp>
#include <origin/algorithm.hpp>
#include <origin/tuple.hpp>

namespace origin
{
  // Random bit generator (concepts)
  // A random bit generator (called uniform random number generator in the
  // standard), generates uniformly distributed sequences of random bits
  // usually as 32 or 64 bit unsigned values.
  //
  // FIXME: Add min/max requirements.
  //
  // FIXME: Rename to Pseudorandom number generator?
  template <typename Gen>
    constexpr bool Random_bit_generator()
    {
      return Has_result_type<Gen>() // Gen::result_type
          && Function<Gen>();       // gen() -> result_type
    }
  

  
  // Random number engine (concept)
  // A random number engine is a random bit generator that provides support
  // for equality comparison, I/O, and seeding.
  //
  // FIXME: Add seeding requirements.
  template <typename Eng>
    constexpr bool Random_number_engine()
    {
      return Random_bit_generator<Eng>()
          && Equality_comparable<Eng>() 
          && Streamable<Eng>();
    }
  

  // FIXME: Implement traits to detect parameter types, etc.
  
  
  // Random number distribution (concept)
  // A random number distribution transforms pseudorandomly generated integers
  // into values described by an associated probability function. This is to
  // say that a histogram of values observed by the random generation of values
  // will respemble the distributions probability function.
  //
  // Note that a random number distribution does not need to generate numbers.
  // For example, we could describe a random graph generator as a multivariate
  // distribution whose parameters include a random number distribution for the
  // graph's order (number of vertices) and another that determines e.g.,
  // the probability of edge connection.
  //
  // FIXME: Ranodm number generators have min/max in addition to the properties
  // of a random value generator. They should also have associated functions
  // like pdf, cdf, mean, variance, etc (the Boost.Math stuff).
  template <typename Dist>
    constexpr bool Random_number_distribution()
    {
      return Equality_comparable<Dist>()
          && Function<Dist, std::minstd_rand&>();
    }

  

  // Random variable (concept)
  // A random variable is a nullary function that generates random values
  // (or variates) from a random number generator and associated probabiltiy
  // distribution. Random variables are typically constructed by binding a 
  // random number engine to a distribution.
  //
  // FIXME: Make sure to add the requirements for v.engine() and 
  // v.distribution().
  template <typename Var>
    constexpr bool Random_variable()
    {
      return Function<Var>() && Has_result_type<Var>();
    }


  
  // Algorithms
  // FIXME: Move these to algorithms.
  
  
  // Random generate (iterator)
  // Fill the objects in [first, last) with values randomly generated by gen
  // and distributed by dist.
  //
  // FIXME: Write requirements
  template <typename I, typename Eng, typename Gen>
    void generate_random(I first, I last, Eng&& eng, Gen&& gen)
    {
      while(first != last) {
        *first = gen(eng);
        ++first;
      }
    }
    
    
    
  // Random generate (range)
  // Fill the objects in range with values randomly generated by gen and
  // distributed by dist.
  //
  // FIXME: Write requirements.
  template <typename R, typename Eng, typename Gen>
    void generate_random(R&& range, Eng&& eng, Gen&& gen)
    {
      generate_random(o_begin(range), o_end(range),
                      std::forward<Eng>(eng),
                      std::forward<Gen>(gen));
    }



  // Default distribution (facility)
  // The default distribution facility provides access to default random
  // distributions for a type. It is used like this:
  //
  //    auto dist = default_distribution<T>()
  //
  // where T is a type that has a default distribution. Note that the default
  // distribution type (decltype(dist)), can also be accessed using:
  //
  //    Default_generator_type<T>
  //
  // FIXME: Rename to default_generator! Only random numbers are distributed
  // in the sense of a probability function. Other value-like elements are
  // distributed in multiple properties.

  

  // Default distribution (traits)
  // The default distribution traits can be used to specialize the default
  // distribution for a type or class of types.
  template <typename T>
    struct default_distribution_traits;

    
  
  // Default distribution (algorithm)
  // Returns the default random value distribution for T.
  template<typename T>
    auto default_distribution() 
      -> decltype(default_distribution_traits<T>::get())
    {
      return default_distribution_traits<T>::get();
    }


  
  // Default distribution type (alais)
  // An alias to the type of the default distribution for the type T.
  template <typename T>
    using Default_distribution_type = 
      decltype(default_distribution_traits<T>::get());



  // Random variable
  // The random variate binds a random variate generator and a random number
  // engine into a nullary function. The argumenty types for Eng and Dist
  // can be given as reference types. 
  template <typename Eng, typename Dist>
    class random_variable
    {
      static_assert(Random_number_engine<Unqualified<Eng>>(), "");
      static_assert(Random_number_distribution<Unqualified<Dist>>(), "");
    public:
      using engine_type = Eng;
      using distribution_type = Dist;
      using result_type = Result_type<Dist>;

      random_variable() = default;
      random_variable(Eng e, Dist d) : eng(e), dist(d) { }

      result_type operator()() { return dist(eng); }

      // FIXME: Do I need const versions of these functions. I can't have them
      // if Eng or Dist has type T&.
      engine_type engine() { return eng; }
      distribution_type distribution() { return dist; }

    private:
      Eng eng;
      Dist dist;
    };

  // Returns a random variable, binding the random number engine, eng, to the
  // specified distribution.
  template <typename Eng, typename Dist>
    inline random_variable<Eng, Dist> make_random(Eng&& eng, Dist&& dist)
    {
      return {eng, dist};
    }

  // Rweturn a random variable that generates default-distributed values of 
  // type T, using the random number engine, eng. The default distribution of
  // T is given by the default_distribution<T>() operation.
  template <typename T, typename Eng>
    inline auto make_random(Eng&& eng)
      -> random_variable<Eng, Default_distribution_type<T>>
    {
      return {eng, default_distribution<T>()};
    }

    
  // Additional random number distributions.


  // Single value distribution
  // A single value generator continuously generates the same value. Note
  // that the value type T must be equality comparable.
  //
  // TODO: This is a special case of random sampling where the sample size is
  // exactly one. Get rid of this class in favor of random sampling.
  template <typename T>
    class single_value_distribution
    {
      static_assert(Equality_comparable<T>(), "");

      using this_type = single_value_distribution<T>;
    public:

      using result_type = T;

      single_value_distribution(const T& x) : value(x) { }

      template <typename Eng>
        const result_type& operator()(Eng& eng) const
        {
          return value;
        }

      // Equality comparable
      bool operator==(const this_type& x) const { return value == x.value; }
      bool operator!=(const this_type & x) const {return value != x.value; }

    private:
      T value;
    };



  // Adapated distribution
  // The adapted generator type is a random value generator that wraps randomly
  // generated values of Dist into the Result type. Note that Result type must 
  // be constructible over the result of Dist.
  template <typename Dist, typename Result>
    class adapted_distribution
    {
      static_assert(Constructible<Result, Result_type<Dist>>(), "");

      using this_type = adapted_distribution<Dist, Result>;
      using wrapped_type = Result_type<Dist>;
    public:
      using result_type = Result;

      adapted_distribution() : dist(default_distribution<wrapped_type>()) { }
      adapted_distribution(const Dist& dist) : dist(dist) { }

      template <typename Eng>
        result_type operator()(Eng& eng)
        {
          return result_type(dist(eng));
        }

      bool operator==(const this_type& x) const { return dist == x.dist; }
      bool operator!=(const this_type& x) const { return dist != x.dist; }

  private:
      Dist dist;
    };



  // Zipf distribution
  //
  // TODO: Implement a zipf distribution to help generate realistic string
  // lengths.
  template <typename T>
    class zipf_distribution
    {
    };
    


  // Random sequence distribution
  // The random sequence distribution creates random sequences of values with 
  // randomly generated size, which is determined by the Len distribution, 
  // whose values are distributed by the Gen distribution.
  //
  // FIXME: The default size distribution should be zipf or zeta.
  template <typename Seq, 
            typename Size = std::uniform_int_distribution<Size_type<Seq>>,
            typename Gen = Default_distribution_type<Value_type<Seq>>>
    class random_sequence_distribution
    {
      using this_type = random_sequence_distribution<Seq, Size, Gen>;
    public:
      using result_type = Seq;

      random_sequence_distribution(const Size& s = Size{0, 32}, 
                                   const Gen& d = Gen{})
        : size{s}, gen{d}
      { }
      
      template <typename Eng>
        Seq operator()(Eng& eng)
        {
          Seq s(size(eng), Value_type<Seq>{});
          // generate_random(s, eng, gen);
          generate(s, make_random(eng, gen));
          return std::move(s);
        }
        
      // Equality comparable
      bool operator==(const this_type& x) const { return equal(x); }
      bool operator!=(const this_type& x) const { return !equal(x); }

    private:
      bool equal(const this_type& x) const
      {
        return size == x.size && gen == x.gen;
      }
      
    private:  
      Size size;
      Gen gen;
    };



  // Random string distribution 
  // The random string distribution creates random strings of with a randomly 
  // generated length, which is determined by the Len distribution, whose
  // characters are distributed by the Alpha distribution.
  //
  // By default, string lengths are uniformly distributed between 0 and 32
  // characters in length, and the characters are uniformly chosen from ASCII 
  // printable characters (32-127).
  template <typename Str, 
            typename Len = std::uniform_int_distribution<Size_type<Str>>,
            typename Alpha = std::uniform_int_distribution<Value_type<Str>>>
    class random_string_distribution
      : public random_sequence_distribution<Str, Len, Alpha>
    {
      using base_type = random_sequence_distribution<Str, Len, Alpha>;
    public:

      random_string_distribution(const Len& l = Len{0, 32},
                                 const Alpha& a = Alpha{33, 126})
        : base_type(l, a)
      { }
    };
    


  // Random iterator distribution
  // The random iterator distribution generates iterators at random positions 
  // in a given container. Positions are uniformly generated.
  //
  // TODO: Should we parameterize over the distribution of positions? That
  // would let us test operations close to the front or back.
  template <typename Cont>
    class random_iterator_distribution
    {
      using this_type = random_iterator_distribution<Cont>;
      using Dist = std::uniform_int_distribution<Size_type<Cont>>;
    public:
      using result_type = Iterator_type<Cont>;

      random_iterator_distribution(Cont& c) 
        : cont(c), dist(0, c.size() - 1)
        { }

      template <typename Eng>
        result_type operator()(Eng& eng)
        {
          return o_next(cont.begin(), dist(eng));
        }

      // Equality comparable
      // Two random iterator generators are equal if they generate iterators
      // into the same container with their positions having the same
      // distribution (currently, that's uniform).
      bool operator==(const this_type& x) const { return &cont == &x.cont; }
      bool operator!=(const this_type & x) const { return &cont != &x.cont; }

    private:
      Cont& cont;
      Dist dist;
    };



  // Random tuple generator
  // The random tuple generator creates random tuples of randomly generated
  // values. Those values are distributed by the distributions over which this
  // template is parameterized.
  template <typename... Dists>
    class random_tuple_generator
    {
      using this_type = random_tuple_generator<Dists...>;
    public:
      using result_type = std::tuple<Result_type<Dists>...>;
      
      random_tuple_generator() : dists{}  { }
      random_tuple_generator(Dists&&... ds) : dists{ds...} { }

      template <typename Eng>
        result_type operator()(Eng& eng)
        {
          return generate<Dists...>(eng);
        }

      // Equality comprable.
      // Two random tuple generators are equal when the their underlying
      // distribution parameters are equal.
      bool operator==(const this_type& x) const { return dists == x.dists; }
      bool operator!=(const this_type& x) const { return dists != x.dists; }

    private:
      // TODO: This seems like a refactorable tuple pattern. What I really want
      // to do is expand the tuple as if it were an expansion pack. Something
      // like this: get(dists)(gen)... Which would expand to:
      //
      //    get<0>(dists)(gen), get<1>(dists)(gen), .... get<N>(dists)(gen)
      //
      // Unfortunately, that is unlikely to happen.
      template <typename D1, typename Eng>
        result_type generate(Eng& eng)
        {
          using std::get;
          return result_type { get<0>(dists(eng)) };
        }

      template <typename D1, typename D2, typename Eng>
        result_type generate(Eng& eng)
        {
          using std::get;
          return result_type { get<0>(dists)(eng), get<1>(dists)(eng) };
        }

      template <typename D1, typename D2, typename D3, typename Eng>
        result_type generate(Eng& eng)
        {
          using std::get;
          return result_type(
            get<0>(dists)(eng), 
            get<1>(dists)(eng), 
            get<2>(dists)(eng)
          );
        }

      std::tuple<Dists...> dists;
    };


  // Default distribution (specializations)

  // Default integral distribution.
  // The default integral distribution is uniformly distributed over the range
  // [0, max] where max is the maximum value of the integral type.
  template <typename T>
    struct default_integral_distribution
    {
      static_assert(Integral<T>(), "");

      static std::uniform_int_distribution<T> get() {return {}; } 
    };



  // Default floating point distribution.
  // The default floating point distribution is uniformly distributed over all 
  // possible values of T.
  template <typename T>
    struct default_floating_point_distribution
    {
      static_assert(Floating_point<T>(), "");

      static std::uniform_real_distribution<T> get() {return {}; }
    };

  
  // Default sequence distribution
  // The default sequence deistribution describes random sequences whose length
  // is Zipf-distributed (with the empty sequence having the highest probability
  // of occurrence) and whose and values are default-distributed according to 
  // the sequence's value type.
  template <typename Seq>
    struct default_sequence_distribution
    {
      static random_sequence_distribution<Seq> get() {return {}; }
    };
    


  namespace traits
  {
    // The arithmetic distribution trait chooses the default distribution among
    // arithmetic types.
    template <typename T, bool = Integral<T>()>
      struct arithmetic_distribution 
        : default_integral_distribution<T>  
      { };

    template <typename T>
      struct arithmetic_distribution<T, false> 
        : default_floating_point_distribution<T> 
      { };



    // For user-defined data types, determine which kind of type we are
    // generating. The kind of type is determined from a sequence of boolean
    // values, only one of which can be true. If T does not match any of these
    // types, then you should expect a compiler error.
    //
    // TODO: Specialize this for more concepts (i.e., Sets and Maps)
    template <typename T, 
              bool = Sequence<T>(), 
              bool = Associative_container<T>()>
      struct udt_distribution;

    template <typename T>
      struct udt_distribution<T, true, false>
        : default_sequence_distribution<T>
      { };



    // The deduce distribution trait tries to deduce an appropriate default
    // distribution for the type T.
    //
    // TODO: It might be nice to have this dispatched on Extended_arithmetic
    // types or something similar since we want to include generators for
    // numeric UDTs.
    template <typename T, bool = Arithmetic<T>()>
      struct deduce_distribution 
        : arithmetic_distribution<T> 
      { };

    template <typename T>
      struct deduce_distribution<T, false>
        : udt_distribution<T>
      { };
  };



  // Default distribution traits
  // The default specialization attempts to deduce the kind of distribution
  // based on known concepts.
  template <typename T>
    struct default_distribution_traits : traits::deduce_distribution<T> { };
  


  // Default distribtution (bool)
  // The default distibution for bool values is a fair Bernoulli trial.
  template <>
    struct default_distribution_traits<bool>
    {
      static std::bernoulli_distribution get() { return {}; }
    };
    


  // Default distribution (string)
  // The default string distribution describes random strings whose lengths
  // are Zipf distributed (with the empty string having the highest probability
  // of occurrence) and whose values are uniformly drawn from the set of
  // printable ASCII characters.
  template <typename C, typename T, typename A>
    struct default_distribution_traits<std::basic_string<C, T, A>>
    {
      using Dist = random_string_distribution<std::basic_string<C, T, A>>;
      
      static Dist get()  { return Dist{}; }
    };

    

  // Default distribution (tuple)
  // The default distribution of a tuple is a generator parameterized over the
  // default distributions of its value types.
  template <typename... Args>
    struct default_distribution_traits<std::tuple<Args...>>
    {
      using Dist = random_tuple_generator<Default_distribution_type<Args>...>;
      
      static Dist get() { return Dist{}; }
    };


/*
  template <> 
    struct default_distribution_traits<char> : integral_distribution_traits<char> { };
  template <> 
    struct default_distribution_traits<signed char> : integral_distribution_traits<signed char> { };
  template <> 
    struct default_distribution_traits<unsigned char> : integral_distribution_traits<unsigned char> { };

  template <> 
    struct default_distribution_traits<short> : integral_distribution_traits<short> { };
  template <> 
    struct default_distribution_traits<int> : integral_distribution_traits<int> { };
  template <> 
    struct default_distribution_traits<long> : integral_distribution_traits<long> { };
  template <> 
    struct default_distribution_traits<long long> : integral_distribution_traits<long long> { };
  
  template <> 
    struct default_distribution_traits<unsigned short> : integral_distribution_traits<unsigned short> { };
  template <> 
    struct default_distribution_traits<unsigned int> : integral_distribution_traits<unsigned int> { };
  template <> 
    struct default_distribution_traits<unsigned long> : integral_distribution_traits<unsigned long> { };
  template <> 
    struct default_distribution_traits<unsigned long long> : integral_distribution_traits<unsigned long long> { };
  
  template <> 
    struct default_distribution_traits<float> : floating_point_distribution_traits<float> { };
  template <> 
    struct default_distribution_traits<double> : floating_point_distribution_traits<double> { };
  template <> 
    struct default_distribution_traits<long double> : floating_point_distribution_traits<long double> { };
  
    
  // Specialization for sequence types
  template <typename T, typename A>
    struct default_distribution_traits<std::vector<T, A>> 
      : sequence_distribution_traits<std::vector<T, A>>
    { };

  template <typename T, typename A>
    struct default_distribution_traits<std::forward_list<T, A>> 
      : sequence_distribution_traits<std::vector<T, A>>
    { };

  template <typename T, typename A>
    struct default_distribution_traits<std::list<T, A>> 
      : sequence_distribution_traits<std::vector<T, A>>
    { };

  template <typename T, typename A>
    struct default_distribution_traits<std::deque<T, A>> 
      : sequence_distribution_traits<std::vector<T, A>>
    { };

  // TODO: Specialize generators for other container types.
  //
  // TODO: Is there some kind of pattern that would allow me to write
  // specializations in terms of concepts instead individually by type?

  */

} // namespace origin

#endif
