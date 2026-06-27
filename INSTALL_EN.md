# Installation Guide (Docker)

This guide explains how to run ConfigScope on an external system.

## Requirements
- Docker Engine / Docker Desktop
- Docker Compose v2 (`docker compose`)
- For the GitHub workflow: GitHub Actions with package write permissions enabled

## API key (recommended)
Set `API_KEY` directly in `docker-compose.yml` or `docker-compose.image.yml`.
Use the same value in request header `X-API-Key`.
If `API_KEY` is empty or missing, API access is open.

No `.env` file is required.

## ScriptAccess Code feature flags
Set the ScriptAccess options directly in the Compose file.

```yaml
SCRIPT_ACCESS_CODE_ENABLED: "true"       # show and enable ScriptAccess Code
SCRIPT_ACCESS_CODE_ENABLED: "false"      # hide and disable ScriptAccess Code
SCRIPT_ACCESS_CODE_API_ENABLED: "true"   # allow the runtime API toggle
SCRIPT_ACCESS_CODE_API_ENABLED: "false"  # disable the runtime API toggle
```

`SCRIPT_ACCESS_CODE_ENABLED` controls the UI/button and `/script_access_code` endpoints.
`SCRIPT_ACCESS_CODE_API_ENABLED` controls the API endpoints used to read or change the ScriptAccess Code status at runtime.

After changing the Compose file, recreate the container:

```bash
docker compose -f docker-compose.image.yml up -d --force-recreate
```

Runtime API toggle, protected by `X-API-Key` when `API_KEY` is configured and only available when `SCRIPT_ACCESS_CODE_API_ENABLED` is `"true"`:

```bash
curl -H "X-API-Key: <your-key>" http://localhost:18080/api/v1/script-access-code/status
curl -X POST http://localhost:18080/api/v1/script-access-code/status \
  -H "X-API-Key: <your-key>" \
  -H "Content-Type: application/json" \
  -d '{"enabled": false}'
```

Runtime API changes are reset to the Docker Compose `SCRIPT_ACCESS_CODE_ENABLED` value after container restart. If `SCRIPT_ACCESS_CODE_API_ENABLED` is `"false"`, the runtime API endpoints return `404`.

## Local start with build from source
Use this variant when Docker should build the image locally from `webapp/Dockerfile`.

```bash
docker compose up -d --build
```

## Start with prebuilt GitHub Container Registry image
Use this variant when the image was built by GitHub Actions and pushed to GitHub Container Registry.

```bash
docker compose -f docker-compose.image.yml up -d
```

Image used by `docker-compose.image.yml`:

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

- `v0.153`
- `sha-<short-sha>`
- `latest` for the current published image
- the Git tag name when a `v*` tag is pushed

`docker-compose.image.yml` uses `ghcr.io/syschelle/export-xml-web:latest` by default. Edit the `image:` line if you want to pin a fixed version tag.

## Open
- http://localhost:18080
- or http://<SERVER-IP>:18080

## API example
```bash
curl -X POST "http://localhost:18080/api/v1/export-xml/analyze" \
  -H "X-API-Key: <your-key>" \
  -F "file=@/path/to/export.xml;type=application/xml"
```

## Stop
```bash
docker compose down
# or, for the image based compose file:
docker compose -f docker-compose.image.yml down
```
