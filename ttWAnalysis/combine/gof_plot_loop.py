import os
import sys

if __name__=='__main__':

  datacarddirs = sys.argv[1:]
  
  for i,datacarddir in enumerate(datacarddirs):
    # run on elementary datacards
    cmd = 'python gof_plot.py' 
    cmd += ' --workspace {}'.format(datacarddir)
    cmd += ' --outputfile {}'.format(os.path.join(datacarddir, 'summary_gof_elementary{}.png'.format(i)))
    cmd += ' --includetags datacard'
    os.system(cmd)
    # run on combined datacards
    cmd = 'python gof_plot.py' 
    cmd += ' --workspace {}'.format(datacarddir)
    cmd += ' --outputfile {}'.format(os.path.join(datacarddir, 'summary_gof_combined.png'))
    cmd += ' --includetags dc_combined'
    os.system(cmd)
    # run on all datacards
    cmd = 'python gof_plot.py' 
    cmd += ' --workspace {}'.format(datacarddir)
    cmd += ' --outputfile {}'.format(os.path.join(datacarddir, 'summary_gof_all.png'))
    os.system(cmd)
