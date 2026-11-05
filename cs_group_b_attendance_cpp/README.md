# CS Group B Attendance (C++ server, no database)
## Important setup before deployment
1. Set `ADMIN_PASSWORD` to a strong private password in your host's environment variables.
2. Set `CENTER_LAT` and `CENTER_LON` to the exact approved face-to-face venue coordinates. Defaults are only placeholders (Accra); do not deploy without changing them.
3. `RADIUS_METERS` defaults to 100.
4. This app saves CSV records in `attendance.csv`. On cloud hosts, attach a persistent disk and set `ATTENDANCE_FILE` to its mount path. Without persistent disk, records may disappear on redeploy/restart.
5. The admin token currently equals the admin password; use HTTPS and a strong password. For a real class deployment, consider stronger authentication/rate limiting.
## Build
```bash
cmake -S . -B build
cmake --build build
./build/server
```
Open http://localhost:8080
## Deploy from GitHub
Push this folder to a GitHub repository, then create a Render Web Service from the repo. Select Docker. Configure environment variables `ADMIN_PASSWORD`, `CENTER_LAT`, `CENTER_LON`, `RADIUS_METERS=100`. Add a persistent disk and set `ATTENDANCE_FILE` to its mount path. Set health check path `/`.
