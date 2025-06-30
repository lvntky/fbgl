# Framebuffer Graphics Library Testing

This directory contains Docker-based testing setup for your Linux framebuffer graphics library. Test your programs in an isolated environment without affecting your main system.

## Quick Start

1. **Setup** (one-time):
   ```bash
   chmod +x test.sh
   ./test.sh build
   ```

2. **Run your tests**:
   ```bash
   ./test.sh run ./your_program
   ```

## Prerequisites

- Docker installed and running
- Your compiled graphics programs in this directory

## Files

- `Dockerfile` - Container configuration with framebuffer support
- `test.sh` - Management script for Docker operations
- Your compiled programs and library files

## Usage

### Build Test Environment
```bash
./test.sh build
```
Creates the Docker image with virtual framebuffer support.

### Run Programs
```bash
# Basic usage
./test.sh run ./my_graphics_test

# With arguments
./test.sh run ./demo --width 800 --height 600

# Full path
./test.sh run /app/bin/test_suite
```

### Management Commands
```bash
./test.sh stop          # Stop any running test
./test.sh clean         # Remove container and image
./test.sh rebuild       # Clean build from scratch
./test.sh shell         # Interactive debugging shell
./test.sh help          # Show all commands
```

## How It Works

1. **Virtual Framebuffer**: Uses `xvfb` to create a virtual display
2. **Isolated Environment**: Your programs run in Ubuntu container
3. **Live Code**: Your directory is mounted, so code changes are immediate
4. **No Graphics Hardware**: No GPU drivers or display required

## Directory Structure

```
your-project/
├── Dockerfile          # Container setup
├── test.sh            # Test runner script
├── README.md          # This file
├── your_library.so    # Your graphics library
├── test_program1      # Compiled test program
├── test_program2      # Another test program
└── src/               # Source code (optional)
```

## Testing Workflow

1. **Develop**: Write and compile your graphics programs
2. **Test**: Run `./test.sh run ./program_name`
3. **Debug**: Use `./test.sh shell` for interactive debugging
4. **Iterate**: Make changes and test again (no rebuild needed)

## Troubleshooting

### Build Issues
```bash
./test.sh rebuild       # Clean rebuild
```

### Permission Problems
```bash
chmod +x test.sh        # Make script executable
chmod +x ./your_program # Make test programs executable
```

### Container Won't Stop
```bash
docker kill fb-test-runner  # Force stop
./test.sh clean            # Clean everything
```

### Debug Inside Container
```bash
./test.sh shell
# Now you're inside the container
ls -la /app
./your_program --debug
exit
```

## Advanced Usage

### Custom Framebuffer Size
Modify the Dockerfile to set specific display dimensions:
```dockerfile
ENV DISPLAY=:99
ENV XVFB_RES=1920x1080x24
```

### Capture Screenshots
Add to your test programs:
```bash
# Inside container
import -window root screenshot.png
```

### Multiple Test Runs
```bash
for test in test_*.exe; do
    ./test.sh run ./$test
done
```

## Performance Notes

- **Fast startup**: Containers start instantly
- **Low overhead**: Minimal resource usage
- **Quick iteration**: Code changes don't require rebuilds
- **Parallel testing**: Can run multiple containers

## Integration with CI/CD

Add to your build pipeline:
```yaml
# Example GitHub Actions
- name: Test Graphics Library
  run: |
    chmod +x test.sh
    ./test.sh build
    ./test.sh run ./test_suite
```

## Cleanup

When done testing:
```bash
./test.sh clean         # Remove everything
# Or just let Docker clean up automatically
```

## Tips

- Keep your test programs in the main directory for easy access
- Use descriptive names for test programs: `test_rendering.exe`, `demo_shapes.exe`
- The container has full Ubuntu tools available for debugging
- Virtual framebuffer works with most graphics operations your library performs
