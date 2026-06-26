# Installation Guide (Docker)

This guide explains how to run the export.xml, Licensefile and Performance Analyzer on an external system.

## Requirements
- Docker Engine / Docker Desktop
- Docker Compose v2 (`docker compose`)
- For the GitHub workflow: GitHub Actions with package write permissions enabled

## API key (recommended)
Set `API_KEY` in `docker-compose.yml` or pass it as an environment variable when using `docker-compose.image.yml`.
Use the same value in request header `X-API-Key`.
If `API_KEY` is empty or missing, API access is open.

## Local start with build from source
Use this variant when Docker should build the image locally from `webapp/Dockerfile`.

```bash
docker compose up -d --build
```

## Start with prebuilt GitHub Container Registry image
Use this variant when the image was built by GitHub Actions and pushed to GitHub Container Registry.

```bash
IMAGE_OWNER=syschelle IMAGE_TAG=latest API_KEY=<your-api-key> docker compose -f docker-compose.image.yml up -d
```

Example image names used by `docker-compose.image.yml`:

```text
ghcr.io/syschelle/export-xml-web:latest
```

## GitHub Actions image build
The workflow is stored here:

```text
.github/workflows/docker-images.yml
```

It builds the Docker image from:

```text
webapp/Dockerfile
```

The workflow pushes these tags to GitHub Container Registry:

- `v0.140`
- `sha-<short-sha>`
- `latest` for the current published image
- the Git tag name when a `v*` tag is pushed

`docker-compose.image.yml` uses `latest` by default. Set `IMAGE_TAG=v0.140` if you want to pin a fixed version.

The default image owner is `syschelle`, matching the current GitHub repository owner. Override `IMAGE_OWNER` only when publishing the image under a different GitHub user or organization.

## Open
- http://localhost:18080
- or http://<SERVER-IP>:18080

## API example
```bash
curl -X POST "http://localhost:18080/api/v1/export-xml/analyze"   -H "X-API-Key: <your-key>"   -F "file=@/path/to/export.xml;type=application/xml"
```

## Stop
```bash
docker compose down
# or, for the image based compose file:
docker compose -f docker-compose.image.yml down
```
