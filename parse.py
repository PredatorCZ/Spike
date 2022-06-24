from hashlib import md5
import json
from ntpath import join
import pathlib
import argparse
import subprocess
import os
import requests

parser = argparse.ArgumentParser()
parser.add_argument('objects', help='Objects where coverage was executed')
parser.add_argument('--ignore-filename-regex', help='Skip source code files with file paths that match the given regular expression', action='append', metavar='REGEXP', default=[])
args = parser.parse_args()

def prepare_payload():
    is_gh_actions = 'true' == os.getenv('GITHUB_ACTIONS', 'false').lower()
    repo_token = os.getenv('COVERALLS_REPO_TOKEN')

    if not repo_token:
        raise RuntimeError('COVERALLS_REPO_TOKEN env/secret was not set')

    output = {
        'source_files': [],
        'repo_token': repo_token,
    }

    git_info = {}
    status = subprocess.run(['git', 'log', '-1', '--pretty=format:%H,%an,%ae,%s'], stdout=subprocess.PIPE)
    if status.returncode > 0:
        print('Could not get git info, trying to use fallbacks')

        if is_gh_actions:
            git_info = {
                'git': {
                    'head': {
                        'id': os.getenv('GITHUB_SHA'),
                        'committer_name': os.getenv('GITHUB_REPOSITORY_OWNER'),
                    },
                    'branch': os.getenv('GITHUB_REF_NAME'),
                }
            }
    else:
        puple = status.stdout.decode('UTF-8').split(',')
        git_info = {
            'git': {
                'head': {
                    'id': puple[0],
                    'committer_name': puple[1],
                    'committer_email': puple[2],
                    'message': ''.join(puple[3:])
                }
            }
        }
    if len(git_info):
        output.update(git_info)

    if is_gh_actions:
        output.update({
            'service_name': 'github-actions-ci',
            'service_number': os.getenv('GITHUB_RUN_NUMBER'),
            'service_job_id': os.getenv('GITHUB_RUN_ID'),
            'service_build_url': '%s/%s/actions/runs/%s' %
                (os.getenv('GITHUB_SERVER_URL'), os.getenv('GITHUB_REPOSITORY'), os.getenv('GITHUB_RUN_ID')),
        })
        output['git']['branch'] = os.getenv('GITHUB_REF_NAME')

    return output

def gather_coverage_profile():
    # gather data
    profiles = pathlib.Path('.').glob('**/*.profraw')

    status = subprocess.run(['llvm-profdata', 'merge'] + [str(p) for p in profiles] + ['-o', 'default.profdata', '--sparse'])
    if status.returncode > 1:
        raise subprocess.CalledProcessError(status.returncode, status.args, status.stdout, status.stderr)

    # generate coverage data
    cov_args = ['llvm-cov', 'export', '--skip-expansions', '--skip-functions', 
        '--instr-profile=' + os.path.abspath('default.profdata'),] + ['--object=' + p for p in args.objects.split(',')] + ['--ignore-filename-regex=' + r for r in args.ignore_filename_regex]

    status = subprocess.run(cov_args, stdout=subprocess.PIPE)
    if status.returncode > 1:
        raise subprocess.CalledProcessError(status.returncode, status.args, status.stdout, status.stderr)

    return json.loads(status.stdout)

class Branch():
    def __init__(self, cov: list):
        self.line_start = cov[0]
        self.col_start = cov[1]
        self.line_end = cov[2]
        self.col_end = cov[3]
        self.exec_count = cov[4]
        self.false_exec_count = cov[5]
        # self.file_id = cov[6]
        # self.expanded_file_id = cov[7]
        # self.kind = cov[8] # 4

    def __eq__(self, other):
        return self.cmp_line(other) and self.cmp_col(other)

    def cmp_col(self, other):
        return self.col_start == other.col_start and self.col_end == other.col_end

    def cmp_line(self, other):
        return self.line_start == other.line_start and self.line_end == other.line_end

    def append_to(self, branches: list, branch_id, pair_id):
        for l in range(self.line_start, self.line_end + 1):
            branches.extend([l, pair_id, branch_id, self.exec_count])
            branches.extend([l, pair_id, branch_id + 1, self.false_exec_count])

class Segment():
    def __init__(self, cov: list):
        self.line = cov[0]
        self.column = cov[1]
        self.exec_count = cov[2]
        self.has_exec = cov[3]
        self.is_region_entry = cov[4]
        self.is_gap = cov[5]
    def __eq__(self, o: int):
        return self.line == o

def convert_profile_to_coverage(main_json, payload):
    if not 'data' in main_json:
        raise RuntimeError('data was not found, invalid report')

    main_data = main_json['data']

    if len(main_data) == 0:
        raise RuntimeError('Expected data')
    elif len(main_data) > 1:
        raise RuntimeError('Expected only one subdata block')

    main_files = main_data[0]['files']

    for f in main_files:
        file_data = {'name': os.path.relpath(f['filename']).replace('\\', '/')}
        local_branches: list = f['branches']

        if len(local_branches):
            branches = []
            # sort by starting line
            local_branches.sort(key= lambda k : k[0])
            merged_branches = []

            # merge duplicate branches
            # caused by template methods
            for b in local_branches:
                mb = Branch(b)
                found = False

                for m in merged_branches:
                    if mb == m:
                        m.exec_count = m.exec_count + mb.exec_count
                        m.false_exec_count = m.false_exec_count + mb.false_exec_count
                        found = True
                        break
                if not found:
                    merged_branches.append(mb)

            cur_line = None
            cur_pair_id = 0
            cur_branch_id = 0

            for b in merged_branches:
                if cur_line != None and cur_line == b.line_start:
                    cur_pair_id = cur_pair_id + 1
                else:
                    cur_line = b.line_start
                    cur_pair_id = 0
                    cur_branch_id = 0

                b.append_to(branches, cur_branch_id, cur_pair_id)
                cur_branch_id = cur_branch_id + 2

            file_data['branches'] = branches

        local_segments: list = f['segments']

        if len(local_segments):
            # sort by line
            local_segments.sort(key= lambda k : k[0])
            merged_segments = []
            last_line = None

            # merge segments by lines
            for s in local_segments:
                ms = Segment(s)
                found = False

                if last_line != None and ms == last_line:
                    last = merged_segments[-1]
                    last.exec_count = last.exec_count + ms.exec_count
                    found = True
                else:
                    last_line = ms.line
                if not found:
                    merged_segments.append(ms)

            if len(merged_segments):
                num_lines = 0

                with open(file_data['name'], 'rb') as fp:
                    buffer = fp.read()
                    num_lines = buffer.count(b'\n')
                    file_data['source_digest'] = md5(buffer).digest().hex()

                coverage = [None] * num_lines

                for s in merged_segments:
                    if s.has_exec and not s.is_gap:
                        coverage[s.line - 1] = s.exec_count

                file_data['coverage'] = coverage

        payload['source_files'].append(file_data)


def send_payload(payload):
    response = requests.post('https://coveralls.io/api/v1/jobs', files={'json_file': json.dumps(payload)})
    print(response.status_code)
    print(response.text)


payload = prepare_payload()
profile_data = gather_coverage_profile()
convert_profile_to_coverage(profile_data, payload)
send_payload(payload)
