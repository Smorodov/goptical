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


#ifndef GOPTICAL_SYSTEM_HXX_
#define GOPTICAL_SYSTEM_HXX_

#include <cassert>

#include "goptical/core/error.hpp"
#include "goptical/core/sys/element.hxx"
#include "goptical/core/sys/container.hxx"
#include "goptical/core/trace/params.hxx"
#include "goptical/core/material/proxy.hxx"

namespace _goptical {

  namespace sys {

    void System::set_entrance_pupil(const std::shared_ptr<Surface> &entrance)
    {
      _entrance = entrance;
      update_version();
    }

    void System::undef_entrance_pupil()
    {
      _entrance.reset();
      update_version();
   }

    void System::set_exit_pupil(const std::shared_ptr<Surface> &exit)
    {
      _exit = exit;
      update_version();
    }

    const Surface & System::get_exit_pupil() const
    {
      if (!_exit)
        throw Error("system has no exit pupil defined");

      return *_exit;
    }

    bool System::has_exit_pupil() const
    {
      return _exit.operator bool();
    }

    bool System::has_entrance_pupil() const
    {
      return _entrance.operator bool();
    }

    const trace::Params & System::get_tracer_params() const
    {
      return _tracer_params;
    }

    trace::Params & System::get_tracer_params()
    {
      update_version();
      return _tracer_params;
    }

    math::Transform<3> * & System::transform_cache_entry(unsigned int from, unsigned int to) const
    {
      auto offset = from * _e_count + to;
      assert(offset < _transform_cache.size());
      return const_cast<math::Transform<3> * &>(_transform_cache[offset]);
    }

    const math::Transform<3> & System::get_transform(const Element &from, const Element &to) const
    {
      math::Transform<3> * &e = transform_cache_entry(from.id(), to.id());

      if (!e)
        transform_cache_update(from, to);

      return *e;
    }

    const math::Transform<3> & System::get_global_transform(const Element &from) const
    {
      math::Transform<3> * &e = transform_cache_entry(from.id(), 0);

      if (!e)
        transform_l2g_cache_update(from);

      return *e;
    }

    const math::Transform<3> & System::get_local_transform(const Element &to) const
    {
      math::Transform<3> * &e = transform_cache_entry(0, to.id());

      if (!e)
        transform_g2l_cache_update(to);

      return *e;
    }

    void System::update_version()
    {
      _version++;
    }

    unsigned int System::get_version() const
    {
      return _version;
    }

    unsigned int System::get_element_count() const
    {
      return _e_count - 1;
    }

    Element & System::get_element(unsigned int index) const
    {
      assert(index > 0 && index <= _e_count);
      return *_index_map[index];
    }

    const std::shared_ptr<material::Base> & System::get_environment() const
    {
      material::Base *base = _env_proxy.get();
      material::Proxy *proxy = dynamic_cast<material::Proxy *> (base);
      if (!proxy)
	throw Error("Invalid proxy type");
      return proxy->get_material();
    }

    const std::shared_ptr<material::Base> & System::get_environment_proxy() const
    {
      return _env_proxy;
    }

  }
}

#endif

