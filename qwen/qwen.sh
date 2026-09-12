#!/bin/bash
set -euo pipefail

# 在这里填写自己的 DeepSeek API Key
API_KEY='sk-0376c7efe52e496db086d15246bc8cce'

if [ "$#" -ne 1 ]; then
    echo "请传入一个问题参数" >&2
    exit 1
fi

system_prompt='你是一个智能音箱助手，回复非常简洁、口语化，适合直接念出来。请遵守以下规则：1.每次回复尽量控制在1-2句话内。2.除了逗号和句号，不要用其他标点符号。3.不要列举项目符号。4.直接回答问题，不要复述用户问题。5.语气亲切自然，像朋友聊天一样。'

payload=$(jq -n \
    --arg system "$system_prompt" \
    --arg question "$1" \
    '{
        model: "deepseek-flash",
        messages: [
            {role: "system", content: $system},
            {role: "user", content: $question}
        ],
        thinking: {type: "disabled"},
        reasoning_effort: "none",
        max_tokens: 256,
        response_format: {type: "text"},
        stream: false
    }')

curl -sS --connect-timeout 15 --max-time 120 \
    'https://api.deepseek.com/chat/completions' \
    -H "Authorization: Bearer ${API_KEY}" \
    -H 'Content-Type: application/json' \
    -H 'Accept: application/json' \
    --data-raw "$payload" |
    jq -c .
