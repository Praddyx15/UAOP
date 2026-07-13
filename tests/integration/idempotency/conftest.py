import os
import uuid

import nats
import pytest_asyncio

NATS_URL = os.environ.get("UAOP_NATS_URL", "nats://localhost:4222")


@pytest_asyncio.fixture
async def jetstream():
    nc = await nats.connect(NATS_URL)
    js = nc.jetstream()
    try:
        yield js
    finally:
        await nc.close()


def unique_suffix() -> str:
    return uuid.uuid4().hex[:8]
