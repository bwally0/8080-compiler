# GitHub Actions Testing Pipeline Setup Guide

This guide will help you integrate automated testing of your BML compiler tests into GitHub Actions.

## Overview

The GitHub Actions pipeline will:
1. Build the BML compiler on every push/PR
2. Run all test files in the `tests/` directory
3. Report pass/fail results
4. Handle both lexer/parser errors and valid compilations

## Step 1: Create the Workflow File

Create a new directory and file in your repository:

```
.github/workflows/test.yml
```

## Step 2: Add the Workflow Configuration

Copy the following content into `.github/workflows/test.yml`:

```yaml
name: Build and Test BML Compiler

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build-and-test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential

    - name: Create build directory
      run: mkdir -p build

    - name: Configure CMake
      working-directory: build
      run: cmake ..

    - name: Build compiler
      working-directory: build
      run: cmake --build .

    - name: Run all tests
      run: |
        EXECUTABLE="./build/bmlc"
        if [ ! -f "$EXECUTABLE" ] && [ -f "./build/bmlc.exe" ]; then
          EXECUTABLE="./build/bmlc.exe"
        fi

        if [ ! -f "$EXECUTABLE" ]; then
          echo "Error: Executable not found"
          exit 1
        fi

        # Test counter
        TOTAL=0
        PASSED=0
        FAILED=0

        # Array of test files to run
        TEST_FILES=(
          "./tests/fib.bml"
          "./tests/factorial.bml"
          "./tests/function_call_errors.bml"
          "./tests/literal_errors.bml"
          "./tests/redeclaration_errors.bml"
          "./tests/semantic_errors.bml"
          "./tests/type_errors.bml"
          "./tests/lexer_errors.bml"
          "./tests/parser_errors.bml"
        )

        echo "========================================="
        echo "Running BML Compiler Tests"
        echo "========================================="
        echo ""

        for test_file in "${TEST_FILES[@]}"; do
          if [ -f "$test_file" ]; then
            TOTAL=$((TOTAL + 1))
            echo "Test $TOTAL: $test_file"
            
            if $EXECUTABLE "$test_file" > /tmp/test_output.txt 2>&1; then
              echo "  ✓ PASSED"
              PASSED=$((PASSED + 1))
            else
              echo "  ✗ FAILED"
              cat /tmp/test_output.txt | sed 's/^/    /'
              FAILED=$((FAILED + 1))
            fi
            echo ""
          fi
        done

        echo "========================================="
        echo "Test Results: $PASSED/$TOTAL passed"
        echo "========================================="

        if [ $FAILED -gt 0 ]; then
          exit 1
        fi
```

## Step 3: Customize Test Files List

The workflow includes an array of test files. Update the `TEST_FILES` array to include:
- **All valid test files** (should compile successfully) - e.g., `fib.bml`, `factorial.bml`
- **All error test files** (should fail with specific errors) - e.g., `lexer_errors.bml`, `parser_errors.bml`

By default, the pipeline treats any non-zero exit code as a failure.

## Step 4: Enhanced Version with Expected Results

For more detailed testing, here's an enhanced version that tracks expected vs actual results:

```yaml
name: Build and Test BML Compiler

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build-and-test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential

    - name: Create build directory
      run: mkdir -p build

    - name: Configure CMake
      working-directory: build
      run: cmake ..

    - name: Build compiler
      working-directory: build
      run: cmake --build .

    - name: Run all tests
      run: bash scripts/run_tests.sh
```

Then create `scripts/run_tests.sh`:

```bash
#!/bin/bash

EXECUTABLE="./build/bmlc"
if [ ! -f "$EXECUTABLE" ] && [ -f "./build/bmlc.exe" ]; then
  EXECUTABLE="./build/bmlc.exe"
fi

if [ ! -f "$EXECUTABLE" ]; then
  echo "Error: Executable not found"
  exit 1
fi

TOTAL=0
PASSED=0
FAILED=0

# Valid tests - should compile/run without errors
VALID_TESTS=(
  "./tests/fib.bml"
  "./tests/factorial.bml"
)

# Error tests - should fail with specific errors
ERROR_TESTS=(
  "./tests/lexer_errors.bml"
  "./tests/parser_errors.bml"
  "./tests/semantic_errors.bml"
  "./tests/type_errors.bml"
)

echo "========================================="
echo "Running Valid Compilation Tests"
echo "========================================="
for test_file in "${VALID_TESTS[@]}"; do
  if [ -f "$test_file" ]; then
    TOTAL=$((TOTAL + 1))
    echo -n "Test $TOTAL ($test_file): "
    
    if $EXECUTABLE "$test_file" > /tmp/test_output.txt 2>&1; then
      echo "✓ PASSED (compiled successfully)"
      PASSED=$((PASSED + 1))
    else
      echo "✗ FAILED (should have compiled)"
      cat /tmp/test_output.txt | sed 's/^/  /'
      FAILED=$((FAILED + 1))
    fi
  fi
done

echo ""
echo "========================================="
echo "Running Error Detection Tests"
echo "========================================="
for test_file in "${ERROR_TESTS[@]}"; do
  if [ -f "$test_file" ]; then
    TOTAL=$((TOTAL + 1))
    echo -n "Test $TOTAL ($test_file): "
    
    if $EXECUTABLE "$test_file" > /tmp/test_output.txt 2>&1; then
      echo "✗ FAILED (should have errored)"
      FAILED=$((FAILED + 1))
    else
      echo "✓ PASSED (error detected)"
      PASSED=$((PASSED + 1))
    fi
  fi
done

echo ""
echo "========================================="
echo "Test Results: $PASSED/$TOTAL passed"
echo "========================================="

if [ $FAILED -gt 0 ]; then
  exit 1
fi
```

## Step 5: Make Script Executable

Add to your git repository and make it executable:

```bash
chmod +x scripts/run_tests.sh
```

## Step 6: Commit and Push

```bash
git add .github/workflows/test.yml scripts/run_tests.sh
git commit -m "Add GitHub Actions testing pipeline"
git push
```

## Step 7: Monitor Results

1. Go to your GitHub repository
2. Click the **Actions** tab
3. Your workflow runs automatically on every push/PR
4. View logs and results for each run

## Customization Options

### Add Branch Protection

To require tests to pass before merging:
1. Go to **Settings** → **Branches**
2. Click **Add rule** under Branch protection rules
3. Require status checks to pass before merging
4. Select "Build and Test BML Compiler"

### Trigger on Schedule

Add cron trigger to test periodically:

```yaml
on:
  schedule:
    - cron: '0 0 * * *'  # Daily at midnight UTC
```

### Add Code Coverage (Optional)

For advanced testing with coverage reports:

```yaml
- name: Generate coverage
  run: |
    sudo apt-get install -y lcov
    # Add coverage flags to CMakeLists.txt compilation
    # Then generate and upload reports
```

### Badge for README

Add this to your README.md to show test status:

```markdown
![Tests](https://github.com/YOUR_USERNAME/8080-compiler/workflows/Build%20and%20Test%20BML%20Compiler/badge.svg)
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Tests fail on Linux but pass locally | Check file paths use `/` not `\` |
| CMake not found | Ensure `sudo apt-get install cmake` is in workflow |
| Permissions denied on script | Run `chmod +x scripts/run_tests.sh` |
| Tests timeout | Increase timeout with `timeout` command or GitHub Actions settings |
| Executable not found | Verify CMakeLists.txt target name matches `bmlc` |

## Next Steps

1. Implement the basic workflow first (Step 2)
2. Test locally with `bash scripts/run_tests.sh` (if using enhanced version)
3. Push to GitHub and verify tests run
4. Add branch protection rules (Step 7)
5. Consider code coverage and additional validations
