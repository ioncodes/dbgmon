#include <iostream>
#include <string>
#include "wrapper.hpp"

struct message
{
	uint32_t process_id;
	uint8_t data[4096 - sizeof(uint32_t)];
};

void* open_event(int access, std::wstring event)
{
	auto handle = wrapper::open_event(
		access,
		false,
		event);
	if (!handle)
	{
		handle = wrapper::create_event(
			nullptr,
			false,
			true,
			event);
	}

	return handle;
}

void* open_mapping(std::wstring mapping)
{
	auto handle = wrapper::open_file_mapping(
		FILE_MAP_READ,
		false,
		mapping);
	if (!handle)
	{
		handle = wrapper::create_file_mapping(
			INVALID_HANDLE_VALUE,
			nullptr,
			PAGE_READWRITE,
			0,
			sizeof(message),
			mapping);
	}

	return handle;
}

int main(int argc, char** argv)
{
	auto buffer_ready = open_event(
		EVENT_ALL_ACCESS,
		L"DBWIN_BUFFER_READY");
	auto data_ready = open_event(
		SYNCHRONIZE,
		L"DBWIN_DATA_READY");
	auto file = open_mapping(
		L"DBWIN_BUFFER");
	auto buffer = (message*)wrapper::map_view_of_file(
		file,
		SECTION_MAP_READ,
		0, 0, 0);

	while (wrapper::wait_for_single_object(
		data_ready,
		INFINITE) == WAIT_OBJECT_0)
	{
		std::cout << "[" << buffer->process_id << "] " << buffer->data;
		wrapper::set_event(buffer_ready);
	}

	wrapper::unmap_view_of_file(buffer);
	wrapper::close_handle(file);
	wrapper::close_handle(buffer_ready);
	wrapper::close_handle(data_ready);

	return 0;
}