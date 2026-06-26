# Installation Guide (Docker)

This guide explains how to run the export.xml, Licensefile and Performance Analyzer on an external system.

## Requirements
- Docker Engine / Docker Desktop
- Docker Compose v2 (`docker compose`)

## API key (recommended)
Set `API_KEY` in `docker-compose.yml` and use the same value in request header `X-API-Key`.
If `API_KEY` is empty or missing, API access is open.

## Start
```bash
docker compose up -d --build
```

## Open
- http://localhost:18080
- or http://<SERVER-IP>:18080

## API example
```bash
curl -X POST "http://localhost:18080/api/v1/export-xml/analyze"   -H "X-API-Key: <your-key>"   -F "file=@/path/to/export.xml;type=application/xml"

# Performance CSV API
curl -X POST "http://localhost:18080/api/v1/performance/analyze"   -H "X-API-Key: <your-key>"   -F "file=@/path/to/performance.csv;text/csv"
```

## Stop
```bash
docker compose down
```
