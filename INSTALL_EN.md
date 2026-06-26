# Installation Guide (Docker)

This guide explains how to run the export.xml, Licensefile and Performance Analyzer on an external system.

## Requirements
- Docker Engine / Docker Desktop
- Docker Compose v2 (`docker compose`)

## Compose files

This project currently provides two Docker Compose files:

- `docker-compose.yml` builds the application image locally from the included `webapp/` source.
- `docker-compose.image.yml` runs a prebuilt image from GitHub Container Registry (`ghcr.io/syschelle/xmlanalyzer:v0.131`).

Status: the prebuilt-image setup is prepared but not released yet. Until the image is published and accessible, use `docker-compose.yml`.

## API key (recommended)
Set `API_KEY` in the Compose file and use the same value in request header `X-API-Key`.
If `API_KEY` is empty or missing, API access is open.

## Start from local source

Use this mode when you received the full deploy bundle including `webapp/`.

```bash
docker compose up -d --build
```

## Start from prebuilt image

Use this mode only after the GHCR image has been released and is accessible.
This mode does not require a local build.

```bash
docker compose -f docker-compose.image.yml pull
docker compose -f docker-compose.image.yml up -d
```

## Open
- http://localhost:18080
- or http://<SERVER-IP>:18080

## API example
```bash
curl -X POST "http://localhost:18080/api/v1/export-xml/analyze"   -H "X-API-Key: <your-key>"   -F "file=@/path/to/export.xml;type=application/xml"

# Performance CSV API
curl -X POST "http://localhost:18080/api/v1/performance/analyze"   -H "X-API-Key: <your-key>"   -F "file=@/path/to/performance.csv;type=text/csv"
```

## Stop

For the local-build Compose file:

```bash
docker compose down
```

For the prebuilt-image Compose file:

```bash
docker compose -f docker-compose.image.yml down
```
