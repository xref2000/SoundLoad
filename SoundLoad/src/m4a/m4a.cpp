#include "pch.hpp"
#include "m4a.hpp"

//
//// TYPES
//

struct remux_ctx_t
{
	AVFormatContext* input  = nullptr;
	AVFormatContext* output = nullptr;
	AVPacket*        packet = nullptr;

	~remux_ctx_t(void)
	{
		av_packet_free(&this->packet);
		avformat_close_input(&this->input);

		if (this->output)
		{
			if (this->output->pb) avio_closep(&this->output->pb);
			avformat_free_context(this->output);
		}
	}
};

//
//// CORE
//

bool media::remux_m4a(const std::wstring& input_path, const std::wstring& output_path)
{
	remux_ctx_t ctx;

	if (avformat_open_input(&ctx.input, std::string(input_path.begin(), input_path.end()).c_str(), nullptr, nullptr) < 0)
	{
		return false;
	}

	if (avformat_find_stream_info(ctx.input, nullptr) < 0)
	{
		return false;
	}

	int audio_index = -1;

	for (uint32_t i = 0; i < ctx.input->nb_streams; ++i)
	{
		const auto* parameters = ctx.input->streams[i]->codecpar;

		if (parameters->codec_type == AVMEDIA_TYPE_AUDIO && parameters->codec_id == AV_CODEC_ID_AAC)
		{
			audio_index = static_cast<int>(i);
			break;
		}
	}

	if (audio_index < 0)
	{
		return false;
	}

	// Write a regular M4A with complete duration and sample/seek tables.

	const std::string output_path_ascii(output_path.begin(), output_path.end());

	if (avformat_alloc_output_context2(&ctx.output, nullptr, "ipod", output_path_ascii.c_str()) < 0)
	{
		return false;
	}

	const auto* input_stream = ctx.input->streams[audio_index];
	auto* output_stream = avformat_new_stream(ctx.output, nullptr);

	if (!output_stream)
	{
		return false;
	}

	if (avcodec_parameters_copy(output_stream->codecpar, input_stream->codecpar) < 0)
	{
		return false;
	}

	output_stream->codecpar->codec_tag = 0;
	output_stream->time_base = input_stream->time_base;

	if (avio_open(&ctx.output->pb, output_path_ascii.c_str(), AVIO_FLAG_WRITE) < 0)
	{
		return false;
	}

	if (avformat_write_header(ctx.output, nullptr) < 0)
	{
		return false;
	}

	ctx.packet = av_packet_alloc();
	if (!ctx.packet)
	{
		return false;
	}

	int64_t timestamp_origin = AV_NOPTS_VALUE;
	uint64_t packet_count = 0;

	int result = 0;

	while ((result = av_read_frame(ctx.input, ctx.packet)) >= 0)
	{
		auto* packet = ctx.packet;
		if (packet->stream_index != audio_index)
		{
			av_packet_unref(packet);
			continue;
		}

		if (packet->flags & AV_PKT_FLAG_CORRUPT)
		{
			return false;
		}

		if (packet->dts == AV_NOPTS_VALUE || packet->pts == AV_NOPTS_VALUE)
		{
			return false;
		}

		// Remove the initial HLS timestamp offset while keeping relative timing.

		if (timestamp_origin == AV_NOPTS_VALUE)
		{
			timestamp_origin = packet->dts;
		}

		packet->dts -= timestamp_origin;
		packet->pts -= timestamp_origin;

		av_packet_rescale_ts(packet, input_stream->time_base, output_stream->time_base);

		packet->stream_index = output_stream->index;
		packet->pos = -1;

		result = av_interleaved_write_frame(ctx.output, packet);
		if (result < 0) return false;

		++packet_count;
	}

	if (result != AVERROR_EOF)
	{
		return false;
	}

	if (!packet_count)
	{
		return false;
	}

	if (av_write_trailer(ctx.output) < 0)
	{
		return false;
	}

	if (avio_closep(&ctx.output->pb) < 0)
	{
		return false;
	}

	return true;
}