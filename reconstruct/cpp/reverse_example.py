import json
import argparse

def main(args):
    with open(args.file, 'r') as f:
        example = json.load(f)

    example['enc_p'] = list(reversed(example['enc_p']))
    example['enc_q'] = list(reversed(example['enc_q']))
    example['enc_d'] = list(reversed(example['enc_d']))
    example['enc_dp'] = list(reversed(example['enc_dp']))
    example['enc_dq'] = list(reversed(example['enc_dq']))
    # example['enc_qp'] = list(reversed(example['enc_qp']))

    with open('reverse_' + args.file, 'w') as f:
        json.dump(example, f)


if __name__ == '__main__':
    default_filename = 'example_128.json'
    arg_parser = argparse.ArgumentParser(description='Reverses encoded RSA parameters in a json file')
    arg_parser.add_argument('--file', nargs='?', const=default_filename,
                            default=default_filename, type=str)
    args = arg_parser.parse_args()
    main(args)