require 'rubygems'
require 'dub'
require 'dub/lua'
require 'pathname'

XML_DOC_PATH  = (Pathname(__FILE__).dirname + '../../doc/xml/').expand_path
BINDINGS_PATH = (Pathname(__FILE__).dirname)

oscit = Dub::Lua.bind Dub.parse(XML_DOC_PATH + "namespaceoscit.xml")[:oscit]

oscit['MidiMessage'].header = 'oscit/midi_message.h'
oscit['MidiMessage'].string_format = "%s"
oscit['MidiMessage'].string_args   = "(*userdata)->to_s().c_str()"

%w{MidiMessage}.each do |name|
  File.open(BINDINGS_PATH + "oscit_#{name}.cpp", 'wb') do |f|
    f.puts oscit[name]
  end
end
