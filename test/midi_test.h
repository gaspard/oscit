/*
  ==============================================================================

   This file is part of the OSCIT library (http://rubyk.org/liboscit)
   Copyright (c) 2007-2010 by Gaspard Bucher - Buma (http://teti.ch).

  ------------------------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

  ==============================================================================
*/

#include "test_helper.h"
#include "oscit/midi_message.h"

class MidiTest : public TestHelper
{
public:
  void test_defaults( void ) {
    MidiMessage m;
    assert_equal(60, m.note());
    assert_equal(80, m.velocity());
    assert_equal(500, m.length());
    assert_equal(1, m.channel());
    assert_equal(0, m.wait());
    assert_equal(NoteOn, m.type());
  }

  void test_create_from_bytes( void ) {
    // 3 bytes midi message (NoteOn, channel 5)
    MidiMessage midi(0x90 + 4, 63, 71, 300);
    assert_equal("MidiMessage +5:Eb3(71), 0/300", midi.to_s());
  }

  void test_create_note( void ) {
    MidiMessage *m = MidiMessage::Note(63, 74, 510);

    assert_equal(63, m->note());
    assert_equal(74, m->velocity());
    assert_equal(510, m->length());
    assert_equal(1, m->channel());
    assert_equal(0, m->wait());
  }

  void test_set_as_note( void ) {
    MidiMessage m;
    m.set_as_note(63);
    assert_equal(63, m.note());
    assert_equal(80, m.velocity());
    assert_equal(500, m.length());
    assert_equal(1, m.channel());
    assert_equal(0, m.wait());

    m.set_as_note(68);
    assert_equal(68, m.note());
    assert_equal(80, m.velocity());
    assert_equal(500, m.length());
    assert_equal(1, m.channel());
    assert_equal(0, m.wait());
  }

  void test_create_ctrl( void ) {
    MidiMessage m;
    m.set_as_ctrl(19, 45);
    assert_equal(19, m.ctrl());
    assert_equal(45, m.value());
    assert_equal(1, m.channel());
    assert_equal(0, m.wait());

    m.set_as_note(12, 55);
    assert_equal(12, m.ctrl());
    assert_equal(55, m.value());
    assert_equal(1, m.channel());
    assert_equal(0, m.wait());
  }

  void test_set_note( void ) {
    MidiMessage m;
    m.set_as_note(212);
    assert_equal(127, m.note());
    m.set_note(-128);
    assert_equal(0, m.note());
    m.set_note(128);
    assert_equal(127, m.note());
    m.set_note(55);
    assert_equal(55, m.note());
  }

  void test_set_velocity( void ) {
    MidiMessage m;
    m.set_as_note(55, 215);
    assert_equal(127, m.velocity());
    m.set_velocity(-128);
    assert_equal(0, m.velocity());
    m.set_velocity(128);
    assert_equal(127, m.velocity());
    m.set_velocity(55);
    assert_equal(55, m.velocity());
  }

  void test_set_length( void ) {
    MidiMessage m;
    m.set_as_note(55, 110, 800);
    assert_equal(800, m.length());
    m.set_length(-10);
    assert_equal(0, m.length());
    m.set_length(125);
    assert_equal(125, m.length());
  }

  void test_set_channel( void ) {
    MidiMessage m;
    m.set_as_note(55, 110, 800, 18);
    assert_equal(16, m.channel());
    m.set_channel(-128);
    assert_equal(1, m.channel());
    m.set_channel(16);
    assert_equal(16, m.channel());
    m.set_channel(0);
    assert_equal(1, m.channel());
    m.set_channel(3);
    assert_equal(3, m.channel());
  }

  void test_to_s( void ) {
    MidiMessage m;
    assert_equal("MidiMessage +1:C3(80), 0/500", m.to_s());
  }
};
