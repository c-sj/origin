// Copyright (c) 2008-2010 Kent State University
// Copyright (c) 2011-2012 Texas A&M University
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

#ifndef ORIGIN_STREAM_STREAM_HPP
#define ORIGIN_STREAM_STREAM_HPP

#include <origin/type/concepts.hpp>
#include <origin/sequence/algorithm.hpp>

namespace origin
{

  //////////////////////////////////////////////////////////////////////////////
  // Input Generator Stream Adaptor
  // 
  // The input generator stream adapts a generator function to an input
  // stream, allowing input values to be generated by writing:
  //
  //    gs >> x
  //
  // where gs is the geneator stream and x is an object of the generated type.
  //
  // Generator streams are, for all intents and purposes, infinite streams.
  // That is, they can always generate successive values.
  //
  // NOTE: The interface of input generator streams is slightly different than
  // that of standard iostreams. Here, we do not define char_type since the
  // stream is not bound to a character device (file, string, terminal, etc).
  // As such, we do not buffer generated values (but we could?). 
  //
  // TODO: Is there such a thing as a generator that can return an error
  // state or stop state? If there is, that is a property of the result type
  // and not the generating function. We should be able to work with such
  // input types (with very interesting results).
  template <typename Gen>
    class igenstream
    {
      static_assert(Generator<Gen>(), "");
    public:
      using value_type = Result_of<Gen()>;
      using size_type = std::size_t;

      igenstream(Gen gen);


      // Return the next value in the stream.
      value_type get();

      // Get the next value in the stream, returning this object.
      igenstream& get(value_type& gen);

      // Get distance(first, last) generated from the stream and return this.
      template <typename I>
        igenstream& get(I first, I last);

      // Get size(range) generated values from the stream and return this.
      template <typename R>
        igenstream& get(R&& range);

      // Ignore n generated values.
      igenstream& ignore(size_type n = 1);


      // Returns true, indicating that another value can always be extracted.
      explicit operator bool() const { return good(); }

      // Returns true. A genator stream is always in a good state.
      bool good() const { return true; }

      // Returns false. A generator stream is always in a good state.
      bool fail() const { return false; }

      // Returns false. A generator stream is always in a good state.
      bool bad() const { return false; }

    private:
      Gen gen;        // The generating function
      value_type cur; // The current value
    };


  template <typename Gen>
    inline
    igenstream<Gen>::igenstream(Gen g)
      : gen(g)
    { }

  template <typename Gen>
    inline auto
    igenstream<Gen>::get() -> value_type
    {
      return gen();
    }

  template <typename Gen>
    inline igenstream<Gen>&
    igenstream<Gen>::get(value_type& x)
    {
      x = get();
      return *this;
    }

  template <typename Gen>
    template <typename I>
      inline igenstream<Gen>&
      igenstream<Gen>::get(I first, I last)
      {
        std::generate(first, last, gen);
        return *this;
      }

  template <typename Gen>
    template <typename R>
      inline igenstream<Gen>&
      igenstream<Gen>::get(R&& range)
      {
        using std::begin;
        using std::end;
        return get(begin(range), end(range));
      }

  template <typename Gen>
    inline igenstream<Gen>&
    igenstream<Gen>::ignore(size_type n)
    {
      while (n != 0) {
        (void)gen();
        --n;
      }
      return *this;
    }


  // Input_streamable
  template <typename Gen>
    inline igenstream<Gen>&
    operator>>(igenstream<Gen>& is, Value_type<igenstream<Gen>>& x)
    {
      return is.get(x);
    }

} // namespace origin

#endif