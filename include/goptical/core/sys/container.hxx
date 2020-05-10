/*

      This file is part of the Goptical Core library.
  
      The Goptical library is free software; you can redistribute it
      and/or modify it under the terms of the GNU General Public
      License as published by the Free Software Foundation; either
      version 3 of the License, or (at your option) any later version.
  
      The Goptical library is distributed in the hope that it will be
      useful, but WITHOUT ANY WARRANTY; without even the implied
      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
      See the GNU General Public License for more details.
  
      You should have received a copy of the GNU General Public
      License along with the Goptical library; if not, write to the
      Free Software Foundation, Inc., 59 Temple Place, Suite 330,
      Boston, MA 02111-1307 USA
  
      Copyright (C) 2010-2011 Free Software Foundation, Inc
      Author: Alexandre Becoulet

*/

#ifndef GOPTICAL_CONTAINER_HXX_
#define GOPTICAL_CONTAINER_HXX_

namespace _goptical {

  namespace sys {

    // Finds an element of type X
    // Looks in the elements and sub elements
    // Returns first match
    template <class X> X* Container::find() const
    {
      for(auto& i : _list)
        {
          X *e;

          if ((e = dynamic_cast<X*>(i.get())))
            return e;

          Container *g;

          if ((g = dynamic_cast<Container*>(i.get())) &&
              (e = g->find<X>()))
            return e;
        }

      return 0;
    }

    template <class X>
    inline void Container::get_elements(const std::function<void (const X &)> &d) const
    {
      for(auto& i : _list)
        {
          X     *e;

          if ((e = dynamic_cast<X*>(i.get())))
            d(*e);

          Container *g;

          if ((g = dynamic_cast<Container*>(i.get())))
            g->get_elements<X>(d);
        }
    }

    template <class X>
    inline void Container::enable_single(const X &e_)
    {
      for(auto& i : _list)
        {
          X     *e;

          if ((e = dynamic_cast<X*>(i.get())))
            e->set_enable_state(e == &e_);

          Container *g;

          if ((g = dynamic_cast<Container*>(i.get())))
            g->enable_single<X>(e_);
        }
    }

    const Container::element_list_t & Container::get_element_list() const
    {
      return _list;
    }

  }
}


#endif

