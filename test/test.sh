#!/bin/bash

IMAGE_NAME="fb-test"
CONTAINER_NAME="fb-test-runner"

show_help() {
    echo "Usage: $0 <command> [program_name]"
    echo ""
    echo "Commands:"
    echo "  build           Build the Docker image"
    echo "  run <program>   Run a program in the container"
    echo "  stop            Stop running container"
    echo "  clean           Remove container and image"
    echo "  rebuild         Clean and build fresh"
    echo "  shell           Start interactive shell in container"
    echo ""
    echo "Examples:"
    echo "  $0 build"
    echo "  $0 run ./my_graphics_test"
    echo "  $0 run /app/test_program arg1 arg2"
    echo "  $0 stop"
    echo "  $0 clean"
}

build_image() {
    echo "Building Docker image..."
    docker build -t $IMAGE_NAME .
    if [ $? -eq 0 ]; then
        echo "✓ Image built successfully"
    else
        echo "✗ Build failed"
        exit 1
    fi
}

run_program() {
    if [ -z "$1" ]; then
        echo "Error: No program specified"
        echo "Usage: $0 run <program> [args...]"
        exit 1
    fi
    
    echo "Running program: $@"
    docker run --rm \
        --name $CONTAINER_NAME \
        -v $(pwd):/app \
        $IMAGE_NAME "$@"
}

stop_container() {
    echo "Stopping container..."
    docker stop $CONTAINER_NAME 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✓ Container stopped"
    else
        echo "No running container found"
    fi
}

clean_all() {
    echo "Cleaning up..."
    
    # Stop container if running
    docker stop $CONTAINER_NAME 2>/dev/null
    
    # Remove container if exists
    docker rm $CONTAINER_NAME 2>/dev/null
    
    # Remove image
    docker rmi $IMAGE_NAME 2>/dev/null
    
    echo "✓ Cleanup complete"
}

rebuild() {
    echo "Rebuilding..."
    clean_all
    build_image
}

start_shell() {
    echo "Starting interactive shell..."
    docker run --rm -it \
        --name $CONTAINER_NAME \
        -v $(pwd):/app \
        --entrypoint /bin/bash \
        $IMAGE_NAME
}

# Main command handling
case "$1" in
    "build")
        build_image
        ;;
    "run")
        shift
        run_program "$@"
        ;;
    "stop")
        stop_container
        ;;
    "clean")
        clean_all
        ;;
    "rebuild")
        rebuild
        ;;
    "shell")
        start_shell
        ;;
    "help"|"-h"|"--help"|"")
        show_help
        ;;
    *)
        echo "Unknown command: $1"
        show_help
        exit 1
        ;;
esac
