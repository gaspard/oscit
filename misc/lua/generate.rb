require 'rubygems'
require 'dub'
require 'dub/lua'
require 'pathname'

XML_DOC_PATH  = (Pathname(__FILE__).dirname + '../../doc/xml/').expand_path
BINDINGS_PATH = (Pathname(__FILE__).dirname)

oscit = Dub::Lua.bind Dub.parse(XML_DOC_PATH + "namespaceoscit.xml")[:oscit]

%w{MidiMessage}.each do |name|
  File.open(BINDINGS_PATH + "oscit_#{name}.cpp", 'wb') do |f|
    f.puts oscit[name]
  end
end
