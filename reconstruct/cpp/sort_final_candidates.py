import json

with open('finalCandidates.json', 'r') as f:
    with open('finalCandidates_sorted.json', 'w') as out:
        finalCandidates = json.load(f)
        json.dump(sorted(finalCandidates['final_candidates'], key=lambda candidate: (int(candidate['d']), int(candidate['p']),
                                                                                     int(candidate['q']), int(candidate['dp']),
                                                                                     int(candidate['dq']))), out, indent=2)
