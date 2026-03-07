#!/bin/bash

BASE_URL="http://localhost:3001/api/engine"

echo "Testing Backend API Endpoints..."

# Health Check
echo "1. Health Check..."
curl -s http://localhost:3001/health

# Note: These will likely 500 if the C engine binary is not at core-engine/build/regm_engine
# but they prove the routes are defined.

echo -e "\n\n2. Testing /uinit..."
curl -s -X POST $BASE_URL/uinit

echo -e "\n\n3. Testing /edit..."
curl -s -X POST -H "Content-Type: application/json" -d '{"paletteId":0,"data":{"modelId":0, "moptionId":1, "parts":[{"variantId":0, "optionId": 0}, {"variantId":-1}, {"variantId":-1}, {"variantId":0}, {"variantId":0, "optionId": 0}, {"variantId":0, "optionId": 0}]}}' $BASE_URL/edit

echo -e "\n\n4. Testing /create..."
curl -s -X POST -H "Content-Type: application/json" -d '{"paletteId":0, "type":0}' $BASE_URL/create

echo -e "\n\n5. Testing /prompt..."
curl -s -X POST -H "Content-Type: application/json" -d '{"paletteId":0, "input":"Create cowboy with knight sword"}' $BASE_URL/prompt

echo -e "\n\nDone."
