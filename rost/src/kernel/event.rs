/**
 * @file kernel/event.rs
 * @version 0.5.0
 *
 * @section License
 * Copyright (C) 2014-2016, Erik Moqvist
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Simba project.
 */

use alloc::boxed::Box;

pub struct Event {
    pub inner: ::Struct_event_t
}

impl Event {

    pub fn new()
               -> Box<Event>
    {
        let mut event: Box<Event> = Box::new(Event { inner: Default::default() });

        unsafe {
            ::event_init(&mut event.inner);
        }

        event
    }
    
    pub fn write(&mut self, buf: &[u8]) -> ::Res
    {
        unsafe {
            Ok(::event_write(&mut self.inner as *mut ::Struct_event_t,
                             buf.as_ptr() as *const i32,
                             buf.len() as u32))
        }
    }

    pub fn read(&mut self, buf: &mut [u8]) -> ::Res
    {
        unsafe {
            Ok(::event_read(&mut self.inner as *mut ::Struct_event_t,
                            buf.as_ptr() as *mut i32,
                            buf.len() as u32))
        }
    }
}

//pub type Event = ::Struct_event_t;

pub trait EventBuffer {
    fn buf_p(&self) -> *mut ::std::os::raw::c_void;
    fn len(&self) -> u32;
}

//impl Event {
//
//    pub fn new()
//               -> Event
//    {
//        let mut event: Event = Default::default();
//
//        unsafe {
//            ::event_init(&mut event);
//        }
//
//        event
//    }
//
//    pub fn write(&mut self, value: u32)
//                 -> ::Res
//    {
//        unsafe {
//            ::event_write(self,
//                          &value as *const _ as *const i32,
//                          4);
//        }
//
//        Ok(0)
//    }
//
//    pub fn read(&mut self, value: &mut u32)
//                -> ::Res
//    {
//        unsafe {
//            ::event_read(self,
//                         value as *mut _ as *mut i32,
//                         4);
//        }
//
//        Ok(0)
//    }
//
//    pub fn size(&mut self)
//                -> ::Res
//    {
//        unsafe {
//            Ok(::event_size(self))
//        }
//    }
//}

impl EventBuffer for u32 {

    fn buf_p(&self) -> *mut ::std::os::raw::c_void
    {
        self as *const _ as *mut ::std::os::raw::c_void
    }

    fn len(&self) -> u32
    {
        4
    }
}

unsafe impl Send for Event {}

impl ::kernel::chan::Channel for Box<Event> {

    fn get_chan_p(&mut self) -> *mut ::std::os::raw::c_void
    {
        &mut self.inner.base as *mut _ as *mut ::std::os::raw::c_void
    }

    fn write(&mut self, buf: &[u8]) -> ::Res
    {
        unsafe {
            self.write(buf)
        }
    }

    fn read(&mut self, buf: &mut [u8]) -> ::Res
    {
        unsafe {
            self.read(buf)
        }
    }
}
