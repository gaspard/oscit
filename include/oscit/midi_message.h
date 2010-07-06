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

#ifndef OSCIT_INCLUDE_OSCIT_MIDI_MESSAGE_H_
#define OSCIT_INCLUDE_OSCIT_MIDI_MESSAGE_H_

#include <vector>
#include <iostream>
#include <sstream>

#include "oscit/reference_counted.h"

namespace oscit {

class Value;

#ifndef uint
typedef unsigned int uint;
#endif

#define MIDI_NOTE_C0 24








/** Midi messages types. */
enum MidiMessageType {   /*   event name        data1               data2              */
  NoteOff        = 0x80, /**< Note Off        | note number       | velocity           */
  NoteOn         = 0x90, /**< Note On         | note number       | velocity           */
  AfterTouch     = 0xA0, /**< Note Aftertouch | note number       | aftertouch value   */
  CtrlChange     = 0xB0, /**< Controller      | cntrl number      | controller value   */
  ProgramChange  = 0xC0, /**< Program Change  | program number    | not used           */
  ChanAftertouch = 0xD0, /**< Ch. Aftertouch  | aftertouch value  | not used           */
  PitchBend      = 0xE0, /**< Pitch Bend      | pitch value (LSB) | pitch value (MSB)  */

  ClockTick      = 0xf8, /**< Tick 24 times for every 1/4 note. */
  ClockStart     = 0xfa, /**< Start sequencer. */
  ClockContinue  = 0xfb, /**< Continue sequencer after 'pause'. */
  ClockStop      = 0xfc, /**< Stop sequencer.  */
  RawMidi        = 0xff,    /**< Other midi message. */
};

/** This class encapsulates midi messages. */
class MidiMessage : public ReferenceCounted {
 public:
  MidiMessage() : type_(RawMidi), wait_(0), length_(0), data_(3) {
    set_as_note(60);
  }

  explicit MidiMessage(unsigned int data_size) : type_(RawMidi), wait_(0), length_(0), data_(data_size) {}

  explicit MidiMessage(const Value &message, time_t wait = 0);

  explicit MidiMessage(char data1, char data2, char data3, float length = 0) : type_(RawMidi), wait_(0), length_(0), data_(3) {
    std::vector<unsigned char> message(3);
    message[0] = data1;
    message[1] = data2;
    message[2] = data3;
    data_   = message;
    length_ = length;
    set_type_from_data();
  }

  virtual ~MidiMessage () {}

  /** Set message from raw midi data. Return false if message could not be set.
   */
  bool unpack(const Value &message, time_t wait = 0);

  /** Set message from raw midi data. Return false if message could not be set.
   */
  bool set(std::vector<unsigned char> &message, time_t wait = 0) {
    if (message.size() == 0) return false;  // no message

    data_ = message;
    wait_ = wait;

    return set_type_from_data();
  }

  /** Create a new midi note on message.
   * @todo optimize object creation with a pool ?
   * @param note the midi note number from 0-127.
   * @param velocity the note velocity from 0-127 (0 = NoteOff).
   * @param length the note length in milliseconds.
   * @param channel the channel number from 1-16.
   * @param wait milliseconds to wait before firing this message.
   * @return new MidiMessage pointer.
   */
  static MidiMessage *Note(int note, int velocity = 80, int length = 500, int channel = 1, time_t wait = 0) {
    MidiMessage *m = new MidiMessage();
    m->set_as_note(note, velocity, length, channel, wait);
    return m;
  }

  /** Set a midi message to NoteOn or NoteOff.
   * @todo optimize object creation with a pool ?
   * @param note the midi note number from 0-127.
   * @param velocity the note velocity from 0-127 (0 = NoteOff).
   * @param length the note length in milliseconds.
   * @param channel the channel number from 1-16.
   * @param wait milliseconds to wait before firing this message.
   * @return new NoteOn MidiMessage.
   */
  void set_as_note(int note, int velocity = 80, int length = 500, int channel = 1, time_t wait = 0) {
    if (velocity != 0) {
      type_ = NoteOn;
    } else {
      type_ = NoteOff;
    }
    set_key(note);
    set_channel(channel);
    set_value(velocity);
    set_length(length);
    wait_   = wait;
  }

  void set_as_ctrl (int ctrl, int ctrl_value,
    unsigned int channel = 1, time_t wait = 0) {
    type_ = CtrlChange;
    set_key(ctrl);
    set_channel(channel);
    set_value(ctrl_value);
    wait_   = wait;
  }

  inline void note_on_to_off() {
    if (type_ == NoteOn) {
      data_[0] -= 0x10;
      type_   = NoteOff;
    }
  }

  void set_type(MidiMessageType type) {
    if (type > 0xf0) {
      data_[0] = (int)type;
    } else {
      data_[0] = (int)type + channel() - 1;
    }
    type_ = type;
  }

  inline void set_note(int note) {
    set_key(note);
  }

  inline void set_ctrl(int ctrl) {
    set_key(ctrl);
  }

  inline void set_key(int note)  {
    if (note > 127) {
      data_[1] = 127;
    } else if (note < 0) {
      data_[1] = 0;
    } else {
      data_[1] = note;
    }
  }

  inline void set_channel(int channel) {
    if (type_ > 0xf0) {
      std::cerr << "Cannot set channel for type " << type_ << "\n";
    } else {
      if (channel > 16) {
        channel = 16;
      } else if (channel < 1) {
        channel = 1;
      }
      data_[0] = type_ + channel - 1;
    }
  }

  inline void set_velocity(int velocity) {
    set_value(velocity);
  }

  inline void set_length(int length) {
    if (length < 0) {
      length_ = 0;
    } else {
      length_ = length;
    }
  }

  inline void set_value(int value) {
    if (value > 127) {
      data_[2] = 127;
    } else if (value < 0) {
      data_[2] = 0;
    } else {
      data_[2] = value;
    }
  }

  inline int type() const { return type_; }

  inline unsigned int note() const { return data_[1]; }

  inline unsigned int ctrl() const { return data_[1]; }

  inline unsigned int value() const { return data_[2]; }

  inline unsigned int channel() const {
    if (type_ > 0xf0) {
      std::cerr << "Cannot get channel for type " << type_ << "\n";
      return 0;
    }

    return data_[0] - type_ + 1;
  }

  inline unsigned int velocity() const { return data_[2]; }

  inline time_t length() const { return length_; }

  inline time_t wait() const { return wait_; }

  inline const std::vector<unsigned char> &data() const { return data_; }

  /** Write the note name (as C2#, D-1, E3) into the buffer. The buffer must be min 5 chars large (C-3#\0). */
  inline void get_note_name(char buffer[]) const {
    unsigned int i = 0;
    int octave = (data_[1] - MIDI_NOTE_C0) / 12;
    int note   = data_[1] % 12;

    if (type_ != NoteOn && type_ != NoteOff) {
      buffer[0] = '?';
      buffer[1] = '?';
      buffer[2] = '\0';
      return;
    }
    switch(note) {
    case 0:
      buffer[i++] = 'C';
      break;
    case 1:
      buffer[i++] = 'C';
      buffer[i++] = '#';
      break;
    case 2:
      buffer[i++] = 'D';
      break;
    case 3:
      buffer[i++] = 'E';
      buffer[i++] = 'b';
      break;
    case 4:
      buffer[i++] = 'E';
      break;
    case 5:
      buffer[i++] = 'F';
      break;
    case 6:
      buffer[i++] = 'F';
      buffer[i++] = '#';
      break;
    case 7:
      buffer[i++] = 'G';
      break;
    case 8:
      buffer[i++] = 'G';
      buffer[i++] = '#';
      break;
    case 9:
      buffer[i++] = 'A';
      break;
    case 10:
      buffer[i++] = 'B';
      buffer[i++] = 'b';
      break;
    case 11:
      buffer[i++] = 'B';
      break;
    default:
      buffer[i++] = '?';
    }

    if (octave < 0) {
      buffer[i++] = '-';
      buffer[i++] = '0' - octave;
    } else {
      buffer[i++] = '0' + octave;
    }
    buffer[i] = '\0';
  }

  /** Return midi message type as a const char. */
  const char * type_name() const {
    switch (type_)
    {
      case NoteOn:
        return "NoteOn";
      case NoteOff:
        return "NoteOff";
      case CtrlChange:
        return "Ctrl";
      case ClockStart:
        return "Start";
      case ClockStop:
        return "Stop";
      case ClockTick:
        return ".";
      case ClockContinue:
        return "Continue";
      default:
        return "?";
    }
  }

  bool operator==(const MidiMessage &other) const {
    return type_    == other.type_ &&
           wait_    == other.wait_ &&
           length_  == other.length_ &&
           data_[0] == other.data_[0] &&
           data_[1] == other.data_[1] &&
           data_[2] == other.data_[2];
  }

  std::string to_s() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
  }

  std::string to_json() const {
    std::ostringstream oss;
    oss << "{\"=m\":[";

    size_t sz = data_.size();
    for(size_t i = 0; i < sz; ++i) {
      if (i != 0) oss << ", ";
      oss << (int)data_[i];
    }

    oss << ", " << length_;
    oss << "]" << "}";

    return oss.str();
  }

 private:
  friend std::ostream &operator<<(std::ostream &out_stream,
    const MidiMessage &midi_message);

  /** Set message type from raw data.
   */
  bool set_type_from_data() {
    if (data_[0] < 0xf0) {
      type_ = data_[0] & 0xf0;
    } else {
      type_ = data_[0];
    }
    return true;
  }

  /** Type of midi messate (NoteOn, NoteOff, etc).
   */
  int type_;

  /** Number of milliseconds (?) to wait before sending the midi event out.
   */
  time_t          wait_;

  /** Duration of note in milliseconds.
   */
  time_t        length_;

  /** Raw midi message as a vector of unsigned char (compatible with RtMidi).
   */
  std::vector<unsigned char> data_;
};

std::ostream &operator<<(std::ostream &out_stream,
  const MidiMessage &midi_message);

}  // oscit

#endif  // OSCIT_INCLUDE_OSCIT_MIDI_MESSAGE_H_
