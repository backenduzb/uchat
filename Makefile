.PHONY: identity education assessment social chat statistics

identity:
	uv run --project services/identity \
		python services/identity/src/manage.py runserver

education:
	uv run --project services/education \
		python services/education/src/manage.py runserver

assessment:
	uv run --project services/assessment \
		python services/assessment/src/manage.py runserver

social:
	uv run --project services/social \
		python services/social/src/manage.py runserver

chat:
	cargo run --manifest-path services/chat/Cargo.toml

statistics:
	go run ./services/statistics/cmd/server