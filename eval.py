
from pathlib import Path
import subprocess
import itertools
from tqdm import tqdm
import time
import os


WORKING_DIR = './render_engine'
EXEC_REL_PATH = '../x64/Release/render_engine.exe'
LOG_FILE_DIR = 'D:/cs348k_eval/clusters2/'
LOG_FILENAME = '{1}_nlights={0:05d}.json'

os.chdir(WORKING_DIR)



NUM_LIGHTS = list(range(0,2001,25)) # list(range(0,1000+1,25))
PIPELINES = [
    #'none',
    #'clay',
    #'deferred-none',
    #'deferred-boundingsphere',
    #'deferred-rastersphere',
    'deferred-clustered-gpu',
    #'forward-none',
    #'forward-boundingsphere',
    'forward-clustered-gpu',
]


if __name__ == '__main__':

    for nlights, pipeline in tqdm(list(itertools.product(NUM_LIGHTS, PIPELINES))):
        log_file = Path(LOG_FILE_DIR) / LOG_FILENAME.format(nlights, pipeline)
        working_dir = f'{Path(WORKING_DIR).absolute()}'.replace('\\', '/')
        command = f'"{EXEC_REL_PATH}" --lights {nlights} --pipeline {pipeline} --eval --log-file "{log_file}"'
        print(command)
        subp = subprocess.Popen(
            command,
            shell=True
        )
        subp.wait()



