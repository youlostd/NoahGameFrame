#!/usr/bin/python
import re
import os
import datetime

regex  = re.compile('(say_reward|say|set_title|say_title|send_letter) *\((".*")\)')
segex  = re.compile('select *\(("[^"]*") *, *("[^"]*")\)')
segex2 = re.compile('".*[^a-zA-Z0-9!\?$%&\*\+#\'\.: \-\)\],;_]"')
cegex  = re.compile('.*\.chat\.("[^"]*").*')
cleanex = re.compile('" *\.\..*\.\. *"')
cleanex2 = re.compile('" *,.*, *"')
max_char = 55

def walk_active(dir):
        """ Parst alle aktiven Quest-Dateien 
        Geht Rekusriv durch alle Unterordner!"""

        basedir = os.path.basename(dir).lower()
        counter = 0

        # Daten aus KOREA und YMIR wollen wir nicht parsen
        # genauso wie versteckte Ordner (beginnen mit '.')
        if ((basedir == 'korea' or basedir == 'ymir') and os.path.isdir(dir)) \
        or ((basedir[0] == '.') and len(basedir) > 1):
                return 0

	for dirname in os.listdir(dir):
		newdir = os.path.join(dir, dirname)
		if os.path.isdir(newdir):
			counter += walk_active(newdir)

	if 'locale_list' in os.listdir(dir):
		locale_list = open(os.path.join(dir, 'locale_list'), 'r')

		for filename in locale_list.read().split('\n'):	
			if len(filename) <= 0:
				continue

	                fulldir = os.path.join(dir, filename)
        	        found = False

                        file = open(fulldir, 'r')
       	                myfile = cleanex.sub('#FUNC#', file.read())
                        myfile = cleanex2.sub('#FUNC2#', myfile)
			file.close()
			
               	        for line in regex.findall(myfile):
                            if len(line[1].strip()) > max_char:
                                if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print 'length --> (' + str(len(line[1].strip())) + ') ' + line[1].strip()
                                counter += 1

                            for line1 in segex2.findall(line[1]):
                       	        if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print line[0] + ' --> ' + line[1]
                                counter += 1

               	        for line1 in cegex.findall(myfile):                            
                            if len(line1.strip()) > max_char:
                       	        if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print 'chat length --> (' + str(len(line1.strip())) + ') ' + line1.strip()
                                counter += 1

                            for line in segex2.findall(line1): 
                                if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print 'chat --> ' + line1
                                counter += 1

               	        for line1 in segex.findall(myfile):
                            if len(line1[0].strip()) > max_char:
                       	        if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print 'select part 1 length --> (' + str(len(line1[0].strip())) + ') ' + line1[0].strip()
                                counter += 1

                            if len(line1[1].strip()) > max_char:
                       	        if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print 'select part 2 length --> (' + str(len(line1[1].strip())) + ') ' + line1[1].strip()
                                counter += 1

                            for line in segex2.findall(line1[0]): 
                                if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print 'select part 1 --> ' + line1[0]
                                counter += 1

                            for line in segex2.findall(line1[1]): 
                                if not found:
					print
                               	        print '# Found in: ' + fulldir
                                found = True
                                print 'select part 2 --> ' + line1[1]
                                counter += 1
		locale_list.close()
	return counter

if __name__ == '__main__':
    #walk_all('.')
    print '## Created: ' + datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print '## Max. characters per line: ' + str(max_char)
    print '## VIM for chat: \.chat\."[^"]*[^a-zA-Z0-9 \.!\?]"'
    errors = walk_active('.')
    print ''
    print '# ' + str(errors) + ' errors found!'
    
