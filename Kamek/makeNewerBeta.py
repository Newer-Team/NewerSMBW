import shutil, os, sys, subprocess




File = open("/Users/Tempus/Projects/Newer/Kamek/src/prolog.S", "rb")
data = File.read()
File.close()


index = data.find("Newer Release Candidate ")
index += 24


data = data[:index] + "{0} - v1{1}".format(sys.argv[1].zfill(2), sys.argv[2].zfill(3)) + data[index+10:]


File = open("/Users/Tempus/Projects/Newer/Kamek/src/prolog.S", "wb")
File.write(data)
File.close()
