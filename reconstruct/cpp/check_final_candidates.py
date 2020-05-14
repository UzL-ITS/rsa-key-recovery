import json
import argparse

def main(args):
    with open(args.finalCandidates, 'r') as f:
        finalCandidates = json.load(f)

    with open(args.reference, 'r') as f:
        reference = json.load(f)

    for candidate in finalCandidates['final_candidates']:
        len_dp = len(bin(int(candidate['dp'], 10))[2:])
        bin_candidate = (bin(int(candidate['dp'], 10))[2:])[::-1]
        bin_reference = ((bin(int(reference['dp'][2:], 10))[2:])[::-1])[:len_dp]

        if bin_candidate == bin_reference:
            print(candidate['dp'])


if __name__ == '__main__':
    default_reference_filename = 'reverse_example_256.json'
    default_final_candidates_filename = 'finalCandidates.json'
    arg_parser = argparse.ArgumentParser(description='Check whether a set of final candidates also contains the correct one.')
    arg_parser.add_argument('--finalCandidates', nargs='?', const=default_final_candidates_filename,
                            default=default_final_candidates_filename, type=str)
    arg_parser.add_argument('--reference', nargs='?', const=default_reference_filename,
                            default=default_reference_filename, type=str)
    args = arg_parser.parse_args()
    main(args)
