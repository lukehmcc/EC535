for i in $(seq 1 5); do
    echo "Welcome to EC535 Fall 2025! Current time and date: $(date -Iseconds)" > /dev/ttyAMA0
    sleep 10
done

