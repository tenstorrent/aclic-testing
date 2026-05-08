# Contributing to ACLIC Testing

Thank you for your interest in contributing to this project! We welcome contributions from the community.

## How to Contribute

### Reporting Bugs

If you find a bug, please report it by [opening an issue](https://github.com/tenstorrent/aclic-testing/issues) on GitHub. Include:

- A clear description of the issue
- Steps to reproduce the problem
- Expected vs. actual behavior
- Your environment (OS, RISC-V toolchain version, Whisper version)

### Submitting Changes

1. **Fork the repository** and create a new branch for your changes
2. **Make your changes** following the coding standards below
3. **Test your changes** by running the test suite with `./run-tests.sh`
4. **Submit a Pull Request** with a clear description of your changes

### Review Process

- Pull requests are reviewed on a **weekly basis**
- We may request changes or clarifications during the review
- Once approved, your PR will be merged by a maintainer

### Coding Standards

- All C code files must include SPDX license headers:
  ```c
  // SPDX-License-Identifier: Apache-2.0
  // SPDX-FileCopyrightText: 2026 Tenstorrent USA, Inc.
  ```
- Follow the existing code style and formatting conventions
- Keep changes focused and avoid unrelated modifications
- Write clear, descriptive commit messages

### Testing Requirements

- All new functionality must include test cases
- Ensure all existing tests pass before submitting your PR
- Test cases should be added to the appropriate test directory

## Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to ospo@tenstorrent.com.

## Questions?

If you have questions about contributing, feel free to [open an issue](https://github.com/tenstorrent/aclic-testing/issues) or contact us at ospo@tenstorrent.com.
