# This script increments the SemVer version number in a text file
# The text file is expected to contain a version number in
# the following format: A.B.C on the first line.
def increaseVersion(file, part)
  f = File.new(file)
  oldreleasetag = f.readline
  f.close
  releasearray = oldreleasetag.split(".")
  case part
  when 'major'
    index = 0
  when 'minor'
    index = 1
  when 'patch'
    index = 2
  else
    raise "Invalid part specified. Allowed values: 'major', 'minor', 'patch'"
  end
  releasearray[index] = (releasearray[index].to_i + 1).to_s
  releasearray.fill('0', index + 1) # Reset lower-order parts to 0
  releasetag = releasearray.join(".")
  f = File.new(file, "w+")
  f.write(releasetag)
  f.close
  puts "Increment #{part} version to #{releasetag} for file #{file}"
  return releasetag
end

if ARGV.length == 2 and File.file?(ARGV[0]) and ['major', 'minor', 'patch'].include?(ARGV[1])
  increaseVersion(ARGV[0], ARGV[1])
else
  puts "Usage: script_name.rb <file_path> <major|minor|patch>"
end
